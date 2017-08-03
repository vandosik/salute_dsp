/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller some definitions                                       *
 *                                                                           *
 *****************************************************************************/

#include <io-pkt/iopkt_driver.h>
#include <stdio.h>
#include <errno.h>
#include <atomic.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/siginfo.h>
#include <sys/syspage.h>
#include <sys/neutrino.h>
#include <sys/mbuf.h>
#include <sys/syslog.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_ether.h>
#include <net/if_media.h>
#include <sys/io-pkt.h>
#include <sys/cache.h>
#include <sys/callout.h>
#include <hw/inout.h>
#include <drvr/mdi.h>
#include <drvr/eth.h>
#include <drvr/nicsupport.h>
#include <hw/nicinfo.h>
#include <sys/device.h>
#include <siglock.h>

#include <dev/mii/miivar.h>

#include <sys/slog.h>
#include <sys/slogcodes.h>

#include "1892vm14_gemac_regs.h"

#include "arm/mc1892vm14_irq.h"

#define SPEED_10    10
#define SPEED_100   100
#define SPEED_1000  1000

#define VM14_NIC_BADDR	0x3800f000
#define VM14_NIC_BSIZE	0x1000

#define REG32(off)		((off) >> 2)
#define REG1(off)		((off) << 2)
#define BIT(bit)		(1U << (bit))
#define RREG(r,o)		((r)->reg[(o)])
#define WREG(r,o,v)		((r)->reg[(o)] = (v))

#define DIR_TX 1
#define DIR_RX 2

#define	VM14_IS_BROADCAST(ptr) \
	((ptr)[0] == 0xff && (ptr)[1] == 0xff && \
	(ptr)[2] == 0xff && (ptr)[3] == 0xff && \
	(ptr)[4] == 0xff && (ptr)[5] == 0xff)

/* Rx and Tx ring descriptors. */
#define DEFAULT_NUM_TX			128
#define DEFAULT_NUM_RX			128
#define DEFAULT_TX_REAP			(DEFAULT_NUM_TX / 4)

#define MIN_TX_REAP				2
#define MAX_TX_REAP				128
#define MIN_NUM_TX				2
#define MIN_NUM_RX				2
#define MAX_NUM_TX				4096
#define MAX_NUM_RX				4096
#define MAX_TX_FRAG				MAX_NUM_TX
#define ETH_NODATA				(sizeof(ether_header_t) + ETH_CRC_LEN)
#define MAX_FRAME_SIZE			(VM14_MAXBUFSIZE-ETH_NODATA)

#define DEFAULT_IPS				0
#define MIN_IPS					80
#define MAX_IPS					5000000
#define IPS2CSR11				(vm14_gemac->ips ? 5000000/vm14_gemac->ips : 0)

#define DEFAULT_TX_THRESHOLD	3
#define MAX_TX_THRESHOLD		4

/* DMA descriptor fields */

#define DMA_RDES0_OWN_BIT		BIT(31)
#define DMA_RDES0_FD			BIT(30)
#define DMA_RDES0_LD			BIT(29)
#define DMA_RDES0_APPST(VAL)	(((VAL) >> 14) & (BIT(15) - 1))
#define DMA_RDES0_ERR_ALIGN		BIT(14)
#define DMA_RDES0_ERR_RUNT		BIT(15)
#define DMA_RDES0_ERR_CRC		BIT(20)
#define DMA_RDES0_ERR_OVER		BIT(21)
#define DMA_RDES0_ERR_JUBBER	BIT(22)
#define DMA_RDES0_ERR_LENGTH	BIT(23)

#define DMA_RDES0_PKTLEN(VAL)	((VAL) & (BIT(14) - 1))

#define DMA_RDES1_EOR			BIT(26)
#define DMA_RDES1_SAC			BIT(25)

#define DMA_TDES0_OWN_BIT		BIT(31)
#define DMA_TDES0_STATUS		(BIT(31) - 1)
#define DMA_TDES1_IOC			BIT(31)
#define DMA_TDES1_LS			BIT(30)
#define DMA_TDES1_FS			BIT(29)
#define DMA_TDES1_EOR			BIT(26)
#define DMA_TDES1_SAC			BIT(25)

#define VM14_DMA_ADDR(x)		x
#define VM14_BUFSIZE(x)		(min((x), VM14_MAXBUFSIZE))
#define VM14_MAXBUFSIZE		(BIT(12) - 1)
struct vm14_gemac_dma_desc {
	volatile uint32_t status;
	volatile uint32_t misc;
	volatile uint32_t buffer1;
	volatile uint32_t buffer2;
} __attribute__((packed));

typedef struct vm14_gemac_dma_desc vm14_tx_desc_t;
typedef struct vm14_gemac_dma_desc vm14_rx_desc_t;

#define VM14_FILTER_LIMIT		64

typedef struct gpio_info {
	uint32_t *base;
	struct gpio_pin_info {
		uint32_t ddr;
		uint32_t ctl;
		uint32_t dr;
		uint8_t  bit;
	} pins[2];
} gpioinfo_t;

typedef struct _nic_vm14_gemac_ext {
	struct ethercom 		ecom; /* common device */
	struct callout			hk_callout; /* house keeping */
	struct callout			mii_callout; /* link up/down */
	nic_config_t			cfg;
	nic_stats_t				stats;
	struct _iopkt_self		*iopkt;
	struct cache_ctrl		cachectl;
	volatile uint32_t		*reg;
	uint32_t				irq_status;
	uint32_t				intrmask;
	void					*pci_dev_hdl;
	int						dying;
	const struct sigevent	* (*isrp)(void *, int);

	int8_t					phy_addr;
	gpioinfo_t				gpio;

	void					*mem_area;
	void					*sd_hook;

	/* TX descriptor and buffer tracking */
	struct evcnt		ev_txdrop __attribute__((aligned (NET_CACHELINE_SIZE)));
	int					num_transmit;	/* No. of TX descriptors cmdline override */
	int					tx_free;
	int					cur_tx_rptr;
	int					cur_tx_wptr;
	vm14_tx_desc_t		*tdesc;
	struct mbuf			**tx_mbuf;
	int					start_running;  // tx in progress

	mdi_t				*mdi;
	int					pkts_received;      // optimization to not probe phy
	int					force_advertise;
	uint32_t			baddr;
	uint32_t			bsize;
	int					tx_reap;
	
	/* RX descriptor and buffer tracking */
	struct _iopkt_inter	inter __attribute__((aligned (NET_CACHELINE_SIZE)));
	int					num_receive;	/* No. of RX descriptors cmdline override */
	int					cur_rx_rptr;
	int					rx_len;
	int					rx_discard;
	vm14_rx_desc_t		*rdesc;
	struct mbuf			**rx_mbuf;
	struct mbuf			*rx_head;
	struct mbuf			**rx_tail;

	int					iid __attribute__((aligned (NET_CACHELINE_SIZE)));

	struct mii_data		bsd_mii;	// for media devctls

	int						sc_type;  // nic hardware family variant
	char*				tmem;   // typed memory area
} vm14_gemac_dev_t;

struct vm14_gemac_dev {
#define __devname__ (vm14_gemac ? vm14_gemac->ecom.ec_if.if_xname : "devnp-vm14-gemac")
	struct device		sc_dev;	/* common device */
	vm14_gemac_dev_t		*sc_vm14_gemac;
	char				filler[sizeof(vm14_gemac_dev_t) + NET_CACHELINE_SIZE];
};

extern void vm14_gemac_mii_callout(void *arg);
int vm14_gemac_findphy (vm14_gemac_dev_t *vm14_gemac);

int vm14_gemac_init_ring(vm14_gemac_dev_t *vm14_gemac);

// hw
extern void vm14_gemac_hw_setmac(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_gemac_hw_speed2tx_threshold(vm14_gemac_dev_t *vm14_gemac, int speed);
extern void vm14_gemac_hw_reset(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_gemac_hw_enable_all_interrupts(vm14_gemac_dev_t *vm14_gemac);
#define IRQ_INIT 0
#define IRQ_DISABLE 1
#define IRQ_ENABLE 2
extern void vm14_gemac_hw_change_interrupts(vm14_gemac_dev_t *vm14_gemac, int type, int state);
extern void vm14_gemac_hw_dma_soft_reset(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_gemac_hw_start(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_gemac_hw_set_tx_dma_addr(vm14_gemac_dev_t *vm14_gemac, uint32_t dma_addr);
extern void vm14_gemac_hw_set_rx_dma_addr(vm14_gemac_dev_t *vm14_gemac, uint32_t dma_addr);
extern void vm14_gemac_hw_stop_tx(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_gemac_hw_stop_rx(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_gemac_hw_set_frame_size(vm14_gemac_dev_t *vm14_gemac, uint32_t frame_size);
extern void vm14_gemac_hw_set_promiscuous_mode(vm14_gemac_dev_t *vm14_gemac, int set);
extern void vm14_gemac_hw_set_allmulticast_mode(vm14_gemac_dev_t *vm14_gemac, int set);
extern void vm14_gemac_hw_setup_hashtable(vm14_gemac_dev_t *vm14_gemac, uint8_t *addr, int set);
extern void vm14_gemac_hwspeed(vm14_gemac_dev_t *vm14_gemac, int speed);
extern void vm14_gemac_hwduplex(vm14_gemac_dev_t *vm14_gemac, int full);
extern uint16_t vm14_gemac_hw_mii_read(void *hdl, uint8_t phy_id, uint8_t reg_num);
extern void vm14_gemac_hw_mii_write(void *hdl, uint8_t phy_id, uint8_t reg_num, uint16_t val);
// extern void vm14_gemac_hw_txstat_wait(vm14_gemac_dev_t *vm14_gemac);
// extern void vm14_gemac_hw_rxstat_wait(vm14_gemac_dev_t *vm14_gemac);
extern uint32_t vm14_gemac_hw_txstat_read(vm14_gemac_dev_t *vm14_gemac, int id);
extern uint32_t vm14_gemac_hw_rxstat_read(vm14_gemac_dev_t *vm14_gemac, int id);
extern void vm14_gemac_hw_dump_registers(vm14_gemac_dev_t *vm14_gemac);
extern int vm14_gemac_hw_phy_reset(vm14_gemac_dev_t *vm14_gemac);

extern int vm14_init_hwi(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_dinit_hwi(vm14_gemac_dev_t *vm14_gemac);

// receive
extern void vm14_gemac_rx(vm14_gemac_dev_t *vm14_gemac, struct nw_work_thread *wtp);
extern void vm14_gemac_update_rx_stats(vm14_gemac_dev_t *vm14_gemac);

// transmit
extern void vm14_gemac_start(struct ifnet *ifp);
extern void vm14_gemac_reap(vm14_gemac_dev_t *vm14_gemac);
extern void vm14_gemac_update_tx_stats(vm14_gemac_dev_t *vm14_gemac);

// devctl
void vm14_gemac_filter(vm14_gemac_dev_t *vm14_gemac/*, int reset*/);
