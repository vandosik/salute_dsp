/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller main routines                                          *
 *                                                                           *
 *****************************************************************************/

#include <io-pkt/iopkt_driver.h>
#include <sys/io-pkt.h>
#include <sys/syspage.h>
#include <sys/device.h>
#include <sys/hwinfo.h>
#include <device_qnx.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_ether.h>
#include <net/if_media.h>
#include <net/if_types.h>
#include <net/if_ether.h>
#include <net/if_media.h>
#include <net/netbyte.h>

#include <drvr/hwinfo.h>
#include <drvr/mdi.h>
#include <drvr/eth.h>
#include <drvr/nicsupport.h>
// #include <drvr/common.h>
#include "1892vm14_gemac.h"

// #define VM14_DEBUG 1

static unsigned char base_ip_addr[6] = { 0, 0, 0, 0, 0, 0 };
int vm14_gemac_entry(void *dll_hdl, struct _iopkt_self *iopkt, char *options);

int vm14_gemac_init(struct ifnet *);
void vm14_gemac_stop(struct ifnet *, int);

int vm14_gemac_ioctl(struct ifnet *, unsigned long, caddr_t);

const struct sigevent * vm14_gemac_isr(void *, int);
int vm14_gemac_process_interrupt(void *, struct nw_work_thread *);
int vm14_gemac_enable_interrupt(void *);

void vm14_gemac_shutdown(void *);
int vm14_gemac_detach_cleanup(vm14_gemac_dev_t *vm14_gemac, int flags);
void bsd_mii_initmedia(vm14_gemac_dev_t *);


struct _iopkt_drvr_entry IOPKT_DRVR_ENTRY_SYM(vm14_gemac) = IOPKT_DRVR_ENTRY_SYM_INIT(vm14_gemac_entry);

#ifdef VARIANT_a
#include <nw_dl.h>
/* This is what gets specified in the stack's dl.c */
struct nw_dll_syms vm14_gemac_syms[] = {
        {"iopkt_drvr_entry", &IOPKT_DRVR_ENTRY_SYM(vm14_gemac)},
        {NULL, NULL}
};
#endif

static char *vm14_gemac_opts[] = {
#define BASE_ADDR 0
	"baddr",
#define TX_REAP 1
	"tx_reap",
#define RECEIVE 2
	"receive",
#define TRANSMIT 3
	"transmit",
#define TYPED_MEM 4
	"typed_mem",
	NULL
};

int vm14_gemac_attach(struct device *, struct device *, void *);
int vm14_gemac_detach(struct device *, int);

CFATTACH_DECL(vm14_gemac,
	sizeof(struct vm14_gemac_dev),
	NULL,
	vm14_gemac_attach,
	vm14_gemac_detach,
	NULL);

struct i_attach_args {
	struct _iopkt_self	*iopkt;
	char				*options;
};

// Get system memory cache attrs
int mem_smart_cache(void)
{
	struct cacheattr_entry	*cache, *base;
	int						i;

	base = SYSPAGE_ENTRY(cacheattr);
	for (i = SYSPAGE_ENTRY(cpuinfo)->data_cache; i != CACHE_LIST_END; i = cache->next) {
		cache = &base[i];
		if (!(cache->flags & CACHE_FLAG_SNOOPED))
			return(0);
	}
	return(1);
}


void
vm14_gemac_hk_callout(void *arg)
{
	vm14_gemac_dev_t			*vm14_gemac   = arg;
	struct ifnet			*ifp      = &vm14_gemac->ecom.ec_if;
	struct _iopkt_self		*iopkt    = vm14_gemac->iopkt;
	struct nw_work_thread	*wtp      = WTP;

	
	// if the transmit side is quiet, process txd descriptors now
	if (!vm14_gemac->start_running) {
		NW_SIGLOCK_P(&ifp->if_snd_ex, iopkt, wtp);
		vm14_gemac_reap(vm14_gemac);
		NW_SIGUNLOCK_P(&ifp->if_snd_ex, iopkt, wtp);
		if (vm14_gemac->cfg.verbose > 12) {
			vm14_gemac_hw_dump_registers(vm14_gemac);
		}
	}
	callout_msec(&vm14_gemac->hk_callout, 10 * 1000, vm14_gemac_hk_callout, vm14_gemac);
}

static int
vm14_gemac_parse_options(vm14_gemac_dev_t *vm14_gemac,
    const char *optstring, nic_config_t *cfg)
{
	char		*value;
	char		*next_value;
	int			opt;
	char		*options, *freeptr;
	char		*c;
	int			rc = 0;

	if (optstring == NULL)
		return 0;

	/* getsubopt() is destructive */
	options = malloc(strlen(optstring) + 1, M_TEMP, M_NOWAIT);
	if (options == NULL)
		return ENOMEM;
	strcpy(options, optstring);
	freeptr = options;

	while (options && *options != '\0') {
		c = options;
		if ((opt = getsubopt(&options, vm14_gemac_opts, &value)) == -1) {
			if (nic_parse_options(cfg, value) == EOK)
				continue;
			goto error;
		}

		if (vm14_gemac == NULL)
			continue;

		switch (opt) {
			case BASE_ADDR:
				vm14_gemac->baddr = VM14_NIC_BADDR;
				vm14_gemac->bsize = VM14_NIC_BSIZE;
				if (value != NULL) {
					next_value = NULL;
					vm14_gemac->baddr = strtoul(value, &next_value, 0);
					if ( next_value != NULL ) {
						value = next_value + 1;
						vm14_gemac->bsize = strtoul(value, &next_value, 0);
					}
				}
				continue;
			case TX_REAP:
				if (value == NULL)
					vm14_gemac->tx_reap = DEFAULT_TX_REAP;
				else
					vm14_gemac->tx_reap = strtoul(value, 0, 0);
				if (vm14_gemac->tx_reap < MIN_TX_REAP)
					vm14_gemac->tx_reap = MIN_TX_REAP;
				else if (vm14_gemac->tx_reap > MAX_TX_REAP)
					vm14_gemac->tx_reap = MAX_TX_REAP;
				continue;
			case RECEIVE:
				if (value == NULL)
					vm14_gemac->num_receive = DEFAULT_NUM_RX;
				else
					vm14_gemac->num_receive = strtoul(value, 0, 0);
				if ( !vm14_gemac->num_receive )
					vm14_gemac->num_receive = DEFAULT_NUM_RX;
				if ( vm14_gemac->num_receive % 4 )
					vm14_gemac->num_receive += (4 - (vm14_gemac->num_receive % 4));
				if ( vm14_gemac->num_receive > MAX_NUM_RX )
					vm14_gemac->num_receive = MAX_NUM_RX;
				if ( vm14_gemac->num_receive < MIN_NUM_RX )
					vm14_gemac->num_receive = MIN_NUM_RX;
				continue;
			case TRANSMIT:
				if (value == NULL)
					vm14_gemac->num_transmit = DEFAULT_NUM_TX;
				else
					vm14_gemac->num_transmit = strtoul(value, 0, 0);
				if ( !vm14_gemac->num_transmit )
					vm14_gemac->num_transmit = DEFAULT_NUM_TX;
				if ( vm14_gemac->num_transmit % 4 )
					vm14_gemac->num_transmit += (4 - (vm14_gemac->num_transmit % 4));
				if ( vm14_gemac->num_transmit > MAX_NUM_TX )
					vm14_gemac->num_transmit = MAX_NUM_TX;
				if ( vm14_gemac->num_transmit < MIN_NUM_TX )
					vm14_gemac->num_transmit = MIN_NUM_TX;
				continue;
			case TYPED_MEM:
				if (value != NULL)
					vm14_gemac->tmem = value;
				continue;
				
		}

error:
		slogf (_SLOGC_NETWORK, _SLOG_WARNING, "%s: unknown option %s", __devname__, c);

		rc = EINVAL;
	}
	
	if (vm14_gemac != NULL) {
		vm14_gemac->tx_reap = vm14_gemac->num_transmit / 4;
		vm14_gemac->tx_reap = vm14_gemac->num_transmit / 4;
		if ( vm14_gemac->tx_reap < MIN_TX_REAP ) {
			vm14_gemac->tx_reap = MIN_TX_REAP;
		} else if ( vm14_gemac->tx_reap > MAX_TX_REAP ) {
			vm14_gemac->tx_reap = MAX_TX_REAP;
		}
	}
	
	free(freeptr, M_TEMP);

	return rc;
}

/*
 * Initial driver entry point.
 */
int
vm14_gemac_entry(void *dll_hdl,  struct _iopkt_self *iopkt, char *options)
{
	int						ret;
	int						instance, single;
	struct device			*dev;
	struct i_attach_args	iargs;
	void					*attach_args;
	nic_config_t			cfg;

	/* parse options */
	memset(&cfg, 0, sizeof(cfg));

	if (options != NULL) {
		if ((ret = vm14_gemac_parse_options(NULL, options, &cfg)) != 0)
			return ret;
	}

	/* do options imply single? */
	single = 0;

	/* initialize to whatever you want to pass to vm14_gemac_attach() */
	attach_args = &iargs;


	memset(&iargs, 0x00, sizeof(iargs));
	iargs.iopkt = iopkt;
	iargs.options = options;

	for (instance = 0;;) {
		/* Apply detection criteria */ /* None */
		dev = NULL; /* No Parent */
		/* Found one */
		if (dev_attach("ag", options, &vm14_gemac_ca, attach_args,
			&single, &dev, NULL) != EOK) {
			break;
		}
		dev->dv_dll_hdl = dll_hdl;
		instance++;

		if (instance || single)
			break;
	}
	if (instance > 0)
		return EOK;

	return ENODEV;
}

static void vm14_gemac_devinfo(vm14_gemac_dev_t *vm14_gemac)
{
	nic_config_t	*cfg = &vm14_gemac->cfg;
	uint64_t		clc = ClockCycles();
	uint32_t		rnd = ((clc >> 32) & 0xFFFFFF) ^ (clc & 0xFFFFFF);
	int				i;

	cfg->vendor_id = 0;
	cfg->device_id = 0;
	cfg->device_revision = 0;
	
	for ( i = 0; i < 6; i++ ) {
		rnd = (0x5BB1A38F * rnd + 0x90BF53CB);
		base_ip_addr[i] = rnd >> 24;
	}
	base_ip_addr[0] &= ~1;
	base_ip_addr[0] |= 2;
	if ( cfg->verbose > 8 ) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: line %d %02X:%02X:%02X:%02X:%02X:%02X", __devname__, __LINE__, base_ip_addr[0], base_ip_addr[1], base_ip_addr[2], base_ip_addr[3], base_ip_addr[4], base_ip_addr[5] );
	}
}

int
vm14_gemac_attach(struct device *parent, struct device *self, void *aux)
{
	int						err;
	struct vm14_gemac_dev		*adev;
	vm14_gemac_dev_t			*vm14_gemac;
	void					*head;
	struct ifnet			*ifp;
	uint8_t					enaddr[ETHER_ADDR_LEN];
	struct i_attach_args	*iargs;
	struct _iopkt_self		*iopkt;
	nic_config_t			*cfg;
	size_t					size;
	int						tmem_fd = NOFD;
	unsigned 				map_flags;
	unsigned				prot_flags;

	/* initialization and attach */
	adev = (struct vm14_gemac_dev *)self;

	iargs = aux;

	iopkt = iargs->iopkt;
	head = adev->filler;
	vm14_gemac = NET_CACHELINE_ALIGN(head);
	adev->sc_vm14_gemac = vm14_gemac;
	vm14_gemac->iopkt = iopkt;
	vm14_gemac->iid = -1; /* Not attached */
	vm14_gemac->rx_tail = &vm14_gemac->rx_head;

	if ( vm14_init_hwi(vm14_gemac) != 0 ) {
		return ENODEV;
	}

	cfg = &vm14_gemac->cfg;
	ifp = &vm14_gemac->ecom.ec_if;
	ifp->if_softc = vm14_gemac;
	strcpy(ifp->if_xname, self->dv_xname);
	cfg->lan = self->dv_unit;

	if ((err = interrupt_entry_init(&vm14_gemac->inter, 0, NULL,
	    IRUPT_PRIO_DEFAULT)) != EOK)
		return err;
	vm14_gemac->inter.func   = vm14_gemac_process_interrupt;
	vm14_gemac->inter.enable = vm14_gemac_enable_interrupt;
	vm14_gemac->inter.arg    = adev;
	/* set capabilities */
// 	ifp->if_capabilities_rx = IFCAP_CSUM_IPv4 | IFCAP_CSUM_TCPv4 | IFCAP_CSUM_UDPv4;
// 	ifp->if_capabilities_tx = IFCAP_CSUM_IPv4 | IFCAP_CSUM_TCPv4 | IFCAP_CSUM_UDPv4;
	ifp->if_capabilities_tx = 0;
	ifp->if_capabilities_rx = 0;


	vm14_gemac->ecom.ec_capabilities |= ETHERCAP_JUMBO_MTU;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	/* Ethernet stats we are interested in */
	vm14_gemac->stats.un.estats.valid_stats =
	    NIC_ETHER_STAT_SINGLE_COLLISIONS |
	    NIC_ETHER_STAT_MULTI_COLLISIONS |
	    NIC_ETHER_STAT_FCS_ERRORS |
	    NIC_ETHER_STAT_ALIGN_ERRORS |
	    NIC_ETHER_STAT_LATE_COLLISIONS |
	    NIC_ETHER_STAT_XCOLL_ABORTED |
	    NIC_ETHER_STAT_INTERNAL_TX_ERRORS |
	    NIC_ETHER_STAT_INTERNAL_RX_ERRORS |
	    NIC_ETHER_STAT_OVERSIZED_PACKETS |
	    NIC_ETHER_STAT_SYMBOL_ERRORS |
	    NIC_ETHER_STAT_JABBER_DETECTED |
	    NIC_ETHER_STAT_SHORT_PACKETS;
	/* Generic networking stats we are interested in */
	vm14_gemac->stats.valid_stats =
	    NIC_STAT_TX_FAILED_ALLOCS | NIC_STAT_RX_FAILED_ALLOCS |
	    NIC_STAT_TXED_MULTICAST | NIC_STAT_TXED_BROADCAST |
	    NIC_STAT_RXED_MULTICAST | NIC_STAT_RXED_BROADCAST;
	/* Parse the options; set up some defaults first */
	cfg->priority = IRUPT_PRIO_DEFAULT;
	cfg->lan = -1;
	cfg->media_rate = -1;
	cfg->duplex = -1;
	vm14_gemac->num_transmit = DEFAULT_NUM_TX;
	vm14_gemac->num_receive  = DEFAULT_NUM_RX;
	cfg->mtu = cfg->mru = ETH_MAX_DATA_LEN;
	cfg->flags |= NIC_FLAG_MULTICAST;
	cfg->revision = NIC_CONFIG_REVISION;
	strcpy ((char *)cfg->uptype, "en");
	// initialize - until mii callback says we have a link ...
    cfg->flags |= NIC_FLAG_LINK_DOWN;
	vm14_gemac->force_advertise = -1;
	vm14_gemac->baddr = VM14_NIC_BADDR;
	vm14_gemac->bsize = VM14_NIC_BSIZE;
	vm14_gemac->tx_reap = DEFAULT_TX_REAP;
	vm14_gemac->tmem = NULL;
	if ((err = vm14_gemac_parse_options(vm14_gemac, iargs->options, cfg)) != 0) {
		slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: error parsing options: %d", __devname__, err);
		vm14_gemac_detach_cleanup(vm14_gemac, 1);
		return EINVAL;
	}
	
	// use low pmem for mbufs by default
	if (vm14_gemac->tmem != NULL && (tmem_fd =  posix_typed_mem_open(vm14_gemac->tmem, O_RDWR, 
									POSIX_TYPED_MEM_ALLOCATE_CONTIG)) == -1) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: unable to open typed memory %s, will not use typed memory", 
													__devname__, vm14_gemac->tmem);
		tmem_fd = NOFD;
	}	
	vm14_gemac->cfg.lan = adev->sc_dev.dv_unit;


	evcnt_attach_dynamic (&vm14_gemac->ev_txdrop, EVCNT_TYPE_MISC, NULL, ifp->if_xname, "txdrop");
	/* Provide our config details, informational only */
	cfg->serial_number = 1;
	cfg->num_irqs = 1;
	if ( cfg->irq[0] == 0 )
		cfg->irq[0] = MC1892VM14_IRQ_EMAC;
	cfg->mem_window_base[0] = vm14_gemac->baddr;
	cfg->mem_window_size[0] = vm14_gemac->bsize;
	cfg->num_mem_windows = 1;

#ifdef USE_LIBCACHE
	if (cache_init(0, &vm14_gemac->cachectl, NULL) == -1) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: cache_init() failed", __devname__);
		err = errno;
		vm14_gemac_detach_cleanup(vm14_gemac, 4);
		return (err);
	}
#endif
		
		
	vm14_gemac->reg = mmap_device_memory(NULL, cfg->mem_window_size[0],
					PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED,
												cfg->mem_window_base[0]);
	
	if ( vm14_gemac->cfg.verbose > 8 )
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: BADDR 0x%08X SIZE 0x%08X REG 0x%08X", __devname__, (uint32_t)cfg->mem_window_base[0], (uint32_t)cfg->mem_window_size[0], (uint32_t)vm14_gemac->reg);
	
	if (vm14_gemac->reg == MAP_FAILED) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: REG failed %d.", __devname__, err);
		vm14_gemac_detach_cleanup(vm14_gemac, 5);
		return errno;
	}
	vm14_gemac_hw_dma_soft_reset(vm14_gemac);
	vm14_gemac_devinfo(vm14_gemac);

	/* Allocate array of mbuf pointers for receiving */
	size = vm14_gemac->num_receive * sizeof(*vm14_gemac->rx_mbuf);
	vm14_gemac->rx_mbuf = malloc(size, M_DEVBUF, M_NOWAIT);

	if (vm14_gemac->rx_mbuf == NULL) {
		err = errno;
		vm14_gemac_detach_cleanup(vm14_gemac, 6);
		return err;
	}
	memset(vm14_gemac->rx_mbuf, 0x00, size);

	/* Allocate array of mbuf pointers for tracking pending transmit packets */
	size = vm14_gemac->num_transmit * sizeof(*vm14_gemac->tx_mbuf);
	vm14_gemac->tx_mbuf = malloc(size, M_DEVBUF, M_NOWAIT);

	if (vm14_gemac->tx_mbuf == NULL) {
		err = errno;
		vm14_gemac_detach_cleanup(vm14_gemac, 7);
		return err;
	}
	memset(vm14_gemac->tx_mbuf, 0x00, size);


	/* Prepare mmap flags */
	if (tmem_fd != NOFD)
		map_flags = MAP_SHARED;
	else
		map_flags = MAP_PRIVATE | MAP_PHYS | MAP_ANON;
	
	if (mem_smart_cache()) {
		prot_flags = PROT_READ | PROT_WRITE;
	} else {
		prot_flags = PROT_READ | PROT_WRITE | PROT_NOCACHE;
	}

	/* Allocate descriptors */
	vm14_gemac->mem_area = mmap(NULL, vm14_gemac->num_receive * sizeof (vm14_rx_desc_t) +
		vm14_gemac->num_transmit * sizeof (vm14_tx_desc_t) + NET_CACHELINE_SIZE,
		prot_flags, map_flags, tmem_fd, 0);


	if (vm14_gemac->mem_area == NULL) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: mmap() of descriptors failed %d.", __devname__, errno);
		vm14_gemac_detach_cleanup(vm14_gemac, 8);
		return errno;
	}

	vm14_gemac->rdesc = (vm14_rx_desc_t *)NET_CACHELINE_ALIGN(vm14_gemac->mem_area);
	vm14_gemac->tdesc = (vm14_tx_desc_t *)(vm14_gemac->rdesc + vm14_gemac->num_receive);
	memset(vm14_gemac->tdesc, 0, sizeof(vm14_tx_desc_t)*vm14_gemac->num_transmit);
	memset(vm14_gemac->rdesc, 0, sizeof(vm14_rx_desc_t)*vm14_gemac->num_receive);

	// hook up so media devctls work
	bsd_mii_initmedia(vm14_gemac);

	/* Set callouts */
	ifp->if_ioctl = vm14_gemac_ioctl;
	ifp->if_start = vm14_gemac_start;
	ifp->if_init = vm14_gemac_init;
	ifp->if_stop = vm14_gemac_stop;
	IFQ_SET_READY(&ifp->if_snd);
	if_attach(ifp);
	{
		int i;
		for (i = 0; i < ETHER_ADDR_LEN; i++) {
			enaddr[i] = base_ip_addr[i];
			cfg->permanent_address[i] = enaddr[i];
		}
	}
	/* Setup the vm14_gemac_dev_t with the info from the options */
	if(memcmp(cfg->current_address, "\0\0\0\0\0\0", 6) == 0) 
		memcpy(cfg->current_address, cfg->permanent_address, ETH_MAC_LEN);
	
	strcpy((char *)cfg->device_description, "1892VM14 GEMAC");
	/* Normal ethernet */
	ether_ifattach(ifp, cfg->current_address);
	/* Set up a timer for housekeeping */
	callout_init(&vm14_gemac->hk_callout);

	if ( cfg->verbose )
		vm14_gemac_hw_dump_registers(vm14_gemac);

	vm14_gemac->sd_hook = shutdownhook_establish(vm14_gemac_shutdown, vm14_gemac);

	if ( vm14_gemac_hw_phy_reset(vm14_gemac) ) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: Phy reset failed", __devname__);
		vm14_gemac_detach_cleanup(vm14_gemac, 9);
		return EIO;
	}

	return EOK;
}

/* Initialize the Rx and Tx rings. */
int
vm14_gemac_init_ring(vm14_gemac_dev_t *vm14_gemac)
{
    int						i;
	struct mbuf				*m;
	off_t					phys;
	struct nw_work_thread	*wtp;

	wtp = WTP;

	vm14_gemac->cur_rx_rptr = 0;
#if VM14_DEBUG > 0
	if (vm14_gemac->cfg.verbose > 12) {
	{

		slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: /* Pre-allocate a receive buffer for each receive descriptor */", __func__);
		slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: vm14_gemac->num_receive %d", __func__, vm14_gemac->num_receive);
	}
#endif
// 	memset(vm14_gemac->tdesc, 0, sizeof(vm14_tx_desc_t) * vm14_gemac->num_transmit);
	for (i = 0; i < vm14_gemac->num_transmit; i++) {
		vm14_gemac->tdesc[i].status = 0;
		vm14_gemac->tdesc[i].misc = DMA_TDES1_SAC;
		vm14_gemac->tdesc[i].buffer1 = 0;
		if ( i > 0 ) {
			phys = drvr_mphys((void *)&vm14_gemac->tdesc[i]);
			vm14_gemac->tdesc[i - 1].buffer2 = VM14_DMA_ADDR(phys);
		}
		if ( i == vm14_gemac->num_transmit - 1 ) {
			phys = drvr_mphys((void *)&vm14_gemac->tdesc[0]);
			vm14_gemac->tdesc[i].buffer2 = VM14_DMA_ADDR(phys);
			vm14_gemac_hw_set_tx_dma_addr(vm14_gemac, phys);
		}
	}
// 	memset(vm14_gemac->rdesc, 0, sizeof(vm14_rx_desc_t) * vm14_gemac->num_receive);
	/* Pre-allocate a receive buffer for each receive descriptor */
	for (i = 0; i < vm14_gemac->num_receive; i++) {
		if (vm14_gemac->rx_mbuf[i] != NULL)
			continue;
		m = m_getcl_wtp(M_DONTWAIT, MT_DATA, M_PKTHDR, wtp);
#if VM14_DEBUG > 0
		if (vm14_gemac->cfg.verbose > 12)
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: m[%d] %p", __func__, i, m);
#endif
		if (m == NULL) {
			i++;
			break;
		}
		vm14_gemac->rx_mbuf[i] = m;
#if VM14_DEBUG > 0
		slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: m[%d] mbuf_phys %llx again", __func__, i, mbuf_phys(m));
#endif
		phys = pool_phys(m->m_data, m->m_ext.ext_page);
#if VM14_DEBUG > 0
		if (vm14_gemac->cfg.verbose > 12)
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: phys %p", __func__, (void *)phys);
#endif
#ifdef USE_LIBCACHE
		CACHE_INVAL(&vm14_gemac->cachectl, m->m_data, phys, m->m_ext.ext_size);
#endif
		vm14_gemac->rdesc[i].status = DMA_RDES0_OWN_BIT;
		vm14_gemac->rdesc[i].misc = DMA_RDES1_SAC | VM14_BUFSIZE(m->m_ext.ext_size);
		vm14_gemac->rdesc[i].buffer1 = VM14_DMA_ADDR(phys);
		if ( i > 0 ) {
			phys = drvr_mphys((void *)&vm14_gemac->rdesc[i]);
			vm14_gemac->rdesc[i - 1].buffer2 = VM14_DMA_ADDR(phys);
		}
		if ( i == vm14_gemac->num_receive - 1 ) {
			phys = drvr_mphys((void *)&vm14_gemac->rdesc[0]);
			vm14_gemac->rdesc[i].buffer2 = VM14_DMA_ADDR(phys);
		}
		
#if VM14_DEBUG > 0
		if (vm14_gemac->cfg.verbose > 12)
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: make recieve buf %d, "
	       		"base = 0x%x , misc = 0x%x status 0x%08X\n", __devname__,
			i, ENDIAN_LE32(vm14_gemac->rdesc[i].buffer1),
			ENDIAN_LE32(vm14_gemac->rdesc[i].misc), vm14_gemac->rdesc[i].status);
#endif
	}
	vm14_gemac_hw_set_rx_dma_addr(vm14_gemac, drvr_mphys((void *)&vm14_gemac->rdesc[0]));

	vm14_gemac->tx_free = vm14_gemac->num_transmit;
	vm14_gemac->cur_tx_wptr = 0;
	vm14_gemac->cur_tx_rptr = 0;
    return 0;
}

int
vm14_gemac_init(struct ifnet *ifp)
{
	int						ret;
	
	vm14_gemac_dev_t			*vm14_gemac;
	struct _iopkt_self		*iopkt;
	struct nw_work_thread	*wtp;

	/*
	 * - enable hardware.
	 *   - look at ifp->if_capenable_[rx/tx]
	 *   - enable promiscuous / multicast filter.
	 * - attach to interrupt.
	 */
	vm14_gemac = (vm14_gemac_dev_t *)ifp->if_softc;
	if (vm14_gemac->dying == 1)
		return 0;
	iopkt = vm14_gemac->iopkt;
	wtp = WTP;

	vm14_gemac_stop(ifp, 0);
	NW_SIGLOCK_P(&ifp->if_snd_ex, iopkt, wtp);
	ifp->if_flags_tx |= IFF_OACTIVE;

	vm14_gemac_hw_reset(vm14_gemac);
	vm14_gemac_init_ring(vm14_gemac);

	if (vm14_gemac->iid == -1) {
		if ((ret = InterruptAttach_r(vm14_gemac->cfg.irq[0], vm14_gemac_isr,
		    vm14_gemac, sizeof(*vm14_gemac), _NTO_INTR_FLAGS_TRK_MSK)) < 0) {
			ret = -ret;
			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "Interrupt attach for %X fail", vm14_gemac->cfg.irq[0]);
			goto do_err;
		}
		vm14_gemac->iid = ret;
	}
	/* Set multicast or promiscuous */
	vm14_gemac_filter(vm14_gemac/*, 0*/);
	if ((vm14_gemac_findphy(vm14_gemac)) == -1) {
		ret = EAGAIN;
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: Failure, unable to detect MII (PHY) Interface.", __devname__);
		goto do_err;
	}
	callout_msec (&vm14_gemac->mii_callout, 3 * 1000, vm14_gemac_mii_callout, vm14_gemac);
// 	// get mtu from stack, mostly for nicinfo
	if ( ifp->if_mtu > MAX_FRAME_SIZE ) {
		ifp->if_mtu = MAX_FRAME_SIZE;
	}
	vm14_gemac->cfg.mtu = ifp->if_mtu;
	vm14_gemac->cfg.mru = ifp->if_mtu;
	vm14_gemac_hw_set_frame_size(vm14_gemac, ifp->if_mtu);
	/* start vm14_gemac */
	vm14_gemac_hw_start(vm14_gemac);
	ifp->if_flags_tx |= IFF_RUNNING;
	ifp->if_flags_tx &= ~IFF_OACTIVE;
	NW_SIGUNLOCK_P(&ifp->if_snd_ex, iopkt, wtp);
	ifp->if_flags |= IFF_RUNNING;
	// start housekeeping callout - give it 10 seconds
	callout_msec(&vm14_gemac->hk_callout, 10 * 1000, vm14_gemac_hk_callout, vm14_gemac);
	return EOK;

do_err:
	ifp->if_flags_tx &= ~IFF_OACTIVE;
	NW_SIGUNLOCK_P(&ifp->if_snd_ex, iopkt, wtp);

	slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: not running.", __devname__);
	return ret;
}

static void
vm14_gemac_rxdrain(vm14_gemac_dev_t *vm14_gemac)
{
	int			i;
	struct mbuf	*m;

	for (i = 0; i < vm14_gemac->num_receive; i++) {
		if ((m = vm14_gemac->rx_mbuf[i]) != NULL) {
			m_freem(m);
			vm14_gemac->rx_mbuf[i] = NULL;
		}
	}

	if ((m = vm14_gemac->rx_head) != NULL) {
		m_freem(m);
		vm14_gemac->rx_len  = 0;
		vm14_gemac->rx_head = NULL;
		vm14_gemac->rx_tail = &vm14_gemac->rx_head;
	}
}

void
vm14_gemac_stop(struct ifnet *ifp, int disable)
{
	struct mbuf				*m;
	int						i;
	vm14_gemac_dev_t			*vm14_gemac = ifp->if_softc;
	struct _iopkt_self		*iopkt = vm14_gemac->iopkt;
	struct nw_work_thread	*wtp = WTP;
	/*
	 * - Cancel any pending io
	 * - Clear any interrupt source registers
	 * - Clear any interrupt pending registers
	 * - Release any queued transmit buffers.
	 */


	/* Shut off the housekeeping & mii callout */
	callout_stop(&vm14_gemac->hk_callout);
	callout_stop(&vm14_gemac->mii_callout);

	if (vm14_gemac->mdi) {
		MDI_DeRegister((mdi_t **)&vm14_gemac->mdi);
		vm14_gemac->mdi = NULL;
	}

	/* Lock out the transmit side */
	NW_SIGLOCK_P(&ifp->if_snd_ex, iopkt, wtp);
	ifp->if_flags_tx |= IFF_OACTIVE;

	vm14_gemac_hw_stop_tx(vm14_gemac);
	/* Release any queued transmit buffers */
	for (i = 0; i < vm14_gemac->num_transmit; i++) {
		if ((m = vm14_gemac->tx_mbuf[i]) != NULL) {
			m_freem(m);
			vm14_gemac->tx_mbuf[i] = NULL;
		}
	}
	ifp->if_flags_tx &= ~(IFF_RUNNING | IFF_OACTIVE);
	/* Done with the transmit side */
	NW_SIGUNLOCK_P(&ifp->if_snd_ex, iopkt, wtp);

	vm14_gemac_hw_stop_rx(vm14_gemac);

	vm14_gemac_hw_dma_soft_reset(vm14_gemac);

	if (disable) {
		if (vm14_gemac->iid != -1) {
			InterruptDetach(vm14_gemac->iid);
			vm14_gemac->iid = -1;
		}
		interrupt_entry_remove(&vm14_gemac->inter, NULL);	/* Must be 'the stack' to call this */
		vm14_gemac_rxdrain(vm14_gemac);
	}

	/* Mark the interface as down */
	ifp->if_flags &= ~(IFF_RUNNING | IFF_OACTIVE);
}

int vm14_gemac_detach_cleanup(vm14_gemac_dev_t *vm14_gemac, int flags)
{
	struct	ifnet		*ifp;

	ifp = &vm14_gemac->ecom.ec_if;

	switch (flags) {
		case	-1:
		/* Don't init() while we're dying. */
			vm14_gemac->dying = 1;
			vm14_gemac_stop (ifp, 1);

			ether_ifdetach (ifp);
			if_detach (ifp);
		case	9:
			munmap(vm14_gemac->mem_area, vm14_gemac->num_receive * sizeof (vm14_rx_desc_t) +
							vm14_gemac->num_transmit * sizeof (vm14_tx_desc_t) + NET_CACHELINE_SIZE);
		case	8:
			free (vm14_gemac->tx_mbuf, M_DEVBUF);

		case	7:
			free (vm14_gemac->rx_mbuf, M_DEVBUF);

		case	6:
			munmap_device_memory ((void *)vm14_gemac->reg, vm14_gemac->cfg.mem_window_size[0]);

		case	5:
			cache_fini (&vm14_gemac->cachectl);

		case	4:
			evcnt_detach (&vm14_gemac->ev_txdrop);

		case	3:
		case	2:
		case	1:
			if (vm14_gemac->sd_hook != NULL)
				shutdownhook_disestablish (vm14_gemac->sd_hook);
		case	0:
			vm14_dinit_hwi(vm14_gemac);
		}

	return EOK;
}

int
vm14_gemac_detach(struct device *dev, int flags)
{
	struct vm14_gemac_dev	*adev;
	vm14_gemac_dev_t		*vm14_gemac;

	adev = (struct vm14_gemac_dev *)dev;
	vm14_gemac = adev->sc_vm14_gemac;

	return vm14_gemac_detach_cleanup(vm14_gemac, -1);
}


void
vm14_gemac_shutdown(void *arg)
{
	vm14_gemac_dev_t		*vm14_gemac;

	/* All of io-pkt is going away.  Just quiet hardware. */

	vm14_gemac = arg;

	vm14_gemac_stop(&vm14_gemac->ecom.ec_if, 1);
}

const struct sigevent *vm14_gemac_isr(void *arg, int iid)
{
	vm14_gemac_dev_t			*vm14_gemac = arg;
	struct _iopkt_inter		*ient;
	uint32_t				status = RREG(vm14_gemac, DMA_STATUS_AND_IRQ);
	uint32_t				irq_status = 0;

	ient = &vm14_gemac->inter;

	if ( status & DMA_STATUS_AND_IRQ_TRANSFER_DONE ) {
		irq_status |= DMA_STATUS_AND_IRQ_TRANSFER_DONE;
	}
	if ( status & DMA_STATUS_AND_IRQ_RECEIVE_DONE ) {
		irq_status |= DMA_STATUS_AND_IRQ_RECEIVE_DONE;
	}
	
	if ( ient->on_list == 0 && !irq_status ) {
		/* IRQ not caused by this card. */
		return NULL;
	}

	vm14_gemac_hw_change_interrupts(vm14_gemac, ~0, IRQ_DISABLE);
#if VM14_DEBUG > 0
	vm14_gemac->stats.un.estats.symbol_errors++;
#endif


	vm14_gemac->iid = iid;

	if ( irq_status ) {
		WREG(vm14_gemac, DMA_STATUS_AND_IRQ, irq_status);	/* Ack interrupts */
	}
	vm14_gemac->irq_status = irq_status;
	return interrupt_queue(vm14_gemac->iopkt, ient);
}

int vm14_gemac_enable_interrupt(void *arg)
{
	struct vm14_gemac_dev		*adev = (struct vm14_gemac_dev *)arg;
	vm14_gemac_dev_t			*vm14_gemac = adev->sc_vm14_gemac;

	vm14_gemac_hw_change_interrupts(vm14_gemac, DMA_INTERRUPT_ENABLE_TRANSMIT_DONE|DMA_INTERRUPT_ENABLE_RECEIVE_DONE, IRQ_INIT);

	return 1;
}

int vm14_gemac_process_interrupt(void *arg, struct nw_work_thread *wtp)
{
	struct ifnet				*ifp;
	struct vm14_gemac_dev		*adev = (struct vm14_gemac_dev *)arg;
	vm14_gemac_dev_t			*vm14_gemac = adev->sc_vm14_gemac;
	uint32_t					status;
	uint32_t					irq_status;

	ifp = &vm14_gemac->ecom.ec_if;
// 	if ( !(ifp->if_flags & IFF_RUNNING) )
// 		return 1;
	irq_status = vm14_gemac->irq_status;
#ifdef VM14_DEBUG
	
	if ( !(ifp->if_flags & IFF_RUNNING) )
		slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: start on stop irq_status %4.4x", __devname__, irq_status);
#endif
	

	do {
		if (vm14_gemac->cfg.verbose > 8)
			slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: interrupt  irq_status=%4.4x\n", __devname__, irq_status);


		if ( (irq_status & DMA_STATUS_AND_IRQ_TRANSFER_DONE) ) { /* Tx-done interrupt */
			if ( !vm14_gemac->start_running ) {
				// if the transmit side is quiet, process txd descriptors now
				NW_SIGLOCK_P(&ifp->if_snd_ex, vm14_gemac->iopkt, wtp);
				if ( vm14_gemac->cfg.verbose > 12 )
					slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: int reap\n", __devname__);
				vm14_gemac_reap(vm14_gemac);
				NW_SIGUNLOCK_P(&ifp->if_snd_ex, vm14_gemac->iopkt, wtp);
			}
		}
		
		/* Rx interrupt */
		if ( irq_status & DMA_STATUS_AND_IRQ_RECEIVE_DONE ) {
			vm14_gemac->pkts_received = 1;  // set activity flag for phy probe
			vm14_gemac_rx(vm14_gemac, wtp);
		}
		
		/* See if we have any more */
		irq_status = 0;
		status = RREG(vm14_gemac, DMA_STATUS_AND_IRQ);
		if ( status & DMA_STATUS_AND_IRQ_TRANSFER_DONE ) {
			irq_status |= DMA_STATUS_AND_IRQ_TRANSFER_DONE;
		}
		if ( status & DMA_STATUS_AND_IRQ_RECEIVE_DONE ) {
			irq_status |= DMA_STATUS_AND_IRQ_RECEIVE_DONE;
		}
		if ( irq_status ) {
			WREG(vm14_gemac, DMA_STATUS_AND_IRQ, irq_status);	/* Ack interrupts */
		}
	} while ( irq_status );

	return 1;
}
