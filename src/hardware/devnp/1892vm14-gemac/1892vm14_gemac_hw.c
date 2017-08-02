/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller hardware part                                          *
 *                                                                           *
 *****************************************************************************/

#include <inttypes.h>
#include <sys/hwinfo.h>
#include <drvr/hwinfo.h>
#include "1892vm14_gemac.h"
#include <arm/mc1892vm14.h>

void vm14_gemac_hw_setmac(vm14_gemac_dev_t *vm14_gemac)
{
	uint8_t		*mac_addr = (uint8_t *)vm14_gemac->cfg.current_address;

	WREG(vm14_gemac, MAC_ADDRESS1_LOW,
			    MAC_ADDRESS1_LOW_SIXTH_BYTE(mac_addr[5]) |
			    MAC_ADDRESS1_LOW_FIFTH_BYTE(mac_addr[4]));

	WREG(vm14_gemac, MAC_ADDRESS1_MED,
			    MAC_ADDRESS1_MED_FOURTH_BYTE(mac_addr[3]) |
			    MAC_ADDRESS1_MED_THIRD_BYTE(mac_addr[2]));

	WREG(vm14_gemac, MAC_ADDRESS1_HIGH,
			    MAC_ADDRESS1_HIGH_SECOND_BYTE(mac_addr[1]) |
			    MAC_ADDRESS1_HIGH_FIRST_BYTE(mac_addr[0]));
}

void vm14_gemac_hw_speed2tx_threshold(vm14_gemac_dev_t *vm14_gemac, int speed)
{
	switch ( speed ) {
		case 100:
			WREG(vm14_gemac, MAC_TRANSMIT_PACKET_START_THRESHOLD, 128);
			break;
		case 1000:
			WREG(vm14_gemac, MAC_TRANSMIT_PACKET_START_THRESHOLD, 1024);
			break;
		default:
			WREG(vm14_gemac, MAC_TRANSMIT_PACKET_START_THRESHOLD, 64);
			break;
	}
}

void vm14_gemac_hw_reset(vm14_gemac_dev_t *vm14_gemac)
{
	uint32_t	reg;

	WREG(vm14_gemac, MAC_ADDRESS_CONTROL, MAC_ADDRESS1_ENABLE);
	WREG(vm14_gemac, MAC_TRANSMIT_FIFO_ALMOST_FULL, (512 - 8));
// 	vm14_gemac_hw_speed2tx_threshold(vm14_gemac, 0);
	WREG(vm14_gemac, MAC_RECEIVE_PACKET_START_THRESHOLD, 64);

// 	reg = RREG(vm14_gemac, MAC_RECEIVE_CONTROL);
// 	reg |= MAC_RECEIVE_CONTROL_STORE_AND_FORWARD;
// 	WREG(vm14_gemac, MAC_RECEIVE_CONTROL, reg);

	reg = RREG(vm14_gemac, DMA_CONFIGURATION);
	reg |= DMA_CONFIGURATION_WAIT_FOR_DONE;
	WREG(vm14_gemac, DMA_CONFIGURATION, reg);

	vm14_gemac_hw_setmac(vm14_gemac);
}

#define CONCATE3(x,y,z) x ## y ## _ ## z
#define GPIO_REG2(id, reg) CONCATE3(MC1892VM14_GPIO, id, reg)
int vm14_init_hwi(vm14_gemac_dev_t *vm14_gemac)
{
	int			i, j;
	unsigned	hwi_off;
	
	hwi_off = hwi_find_device("arasan-gemac", 0);
	if(hwi_off != HWI_NULL_OFF) {
		hwi_tag *tag = hwi_tag_find(hwi_off, HWI_TAG_NAME_nicaddr, 0);
		
		if ( tag ) {
			vm14_gemac->gpio.base = (uint32_t *)mmap_device_memory(NULL, MC1892VM14_GPIO_SIZE,
					PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED,
												MC1892VM14_GPIO_BASE);

			if (vm14_gemac->gpio.base == MAP_FAILED) {
				return -1;
			}
#if VM14_DEBUG > 0
			slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: el24om phy-reset-gpios nicaddr %02x:%02x:%02x:%02x:%02x:%02x\n", __devname__,
					tag->nicaddr.addr[0],
					tag->nicaddr.addr[1],
					tag->nicaddr.addr[2],
					tag->nicaddr.addr[3],
					tag->nicaddr.addr[4],
					tag->nicaddr.addr[5]
			);
#endif
			for ( i = 0, j = 0; j < 2; i += 3, j++ ) {
#if VM14_DEBUG > 0
				slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: el24om phy-reset-gpios nicaddr[%d] %02x\n", __devname__, i, tag->nicaddr.addr[i]);
#endif
				switch ( tag->nicaddr.addr[i] ) {
					case 'A':
						vm14_gemac->gpio.pins[j].ddr = GPIO_REG2(A,DDR) >> 2;
						vm14_gemac->gpio.pins[j].ctl = GPIO_REG2(A,CTL) >> 2;
						vm14_gemac->gpio.pins[j].dr  = GPIO_REG2(A,DR) >> 2;
						break;
					case 'B':
						vm14_gemac->gpio.pins[j].ddr = GPIO_REG2(B,DDR) >> 2;
						vm14_gemac->gpio.pins[j].ctl = GPIO_REG2(B,CTL) >> 2;
						vm14_gemac->gpio.pins[j].dr  = GPIO_REG2(B,DR) >> 2;
						break;
					case 'C':
						vm14_gemac->gpio.pins[j].ddr = GPIO_REG2(C,DDR) >> 2;
						vm14_gemac->gpio.pins[j].ctl = GPIO_REG2(C,CTL) >> 2;
						vm14_gemac->gpio.pins[j].dr  = GPIO_REG2(C,DR) >> 2;
						break;
					case 'D':
						vm14_gemac->gpio.pins[j].ddr = GPIO_REG2(D,DDR) >> 2;
						vm14_gemac->gpio.pins[j].ctl = GPIO_REG2(D,CTL) >> 2;
						vm14_gemac->gpio.pins[j].dr  = GPIO_REG2(D,DR) >> 2;
						break;
					default:
						if ( i == 0 ) {
							return -1;
						}
						vm14_gemac->gpio.pins[j].ddr =
						vm14_gemac->gpio.pins[j].ctl =
						vm14_gemac->gpio.pins[j].dr  = 0;
						break;
				}
				vm14_gemac->gpio.pins[j].bit = tag->nicaddr.addr[i + 1];
#if VM14_DEBUG > 0
				slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: el24om phy-reset-gpios bit[%d] %02x\n", __devname__, j, vm14_gemac->gpio.pins[j].bit);
#endif
			}
		}
		vm14_gemac->phy_addr = hwitag_find_phyaddr(hwi_off, NULL);
	}
	return 0;
}

void vm14_dinit_hwi(vm14_gemac_dev_t *vm14_gemac)
{
	if ( vm14_gemac->gpio.base != MAP_FAILED && vm14_gemac->gpio.base != NULL ) {
		munmap_device_memory(vm14_gemac->gpio.base, MC1892VM14_GPIO_SIZE);
	}
}

int vm14_gemac_hw_phy_reset(vm14_gemac_dev_t *vm14_gemac)
{
	if ( vm14_gemac->gpio.pins[0].bit != 0xFF ) {
		// OUT
		vm14_gemac->gpio.base[vm14_gemac->gpio.pins[0].ddr] |= BIT(vm14_gemac->gpio.pins[0].bit);
		// SOFT
		vm14_gemac->gpio.base[vm14_gemac->gpio.pins[0].ctl] &= ~BIT(vm14_gemac->gpio.pins[0].bit);
		// RESET
		vm14_gemac->gpio.base[vm14_gemac->gpio.pins[0].dr] &= ~BIT(vm14_gemac->gpio.pins[0].bit);
		delay(20);
		vm14_gemac->gpio.base[vm14_gemac->gpio.pins[0].dr] |= BIT(vm14_gemac->gpio.pins[0].bit);
	}
	return 0;
}

void vm14_gemac_hw_change_interrupts(vm14_gemac_dev_t *vm14_gemac, int type, int state)
{
	uint32_t	reg;
	if ( !type ) {
		return;
	}

	reg = RREG(vm14_gemac, DMA_INTERRUPT_ENABLE);
	switch ( state ) {
		case IRQ_INIT:
			WREG(vm14_gemac, DMA_INTERRUPT_ENABLE, 0);
		case IRQ_ENABLE:
			WREG(vm14_gemac, DMA_INTERRUPT_ENABLE, reg | type);
			break;
		case IRQ_DISABLE:
			WREG(vm14_gemac, DMA_INTERRUPT_ENABLE, reg & ~type);
			break;
	}
}

void vm14_gemac_hw_dma_soft_reset(vm14_gemac_dev_t *vm14_gemac)
{
	WREG(vm14_gemac, DMA_CONFIGURATION, DMA_CONFIGURATION_SOFT_RESET);
	delay(10);
	WREG(vm14_gemac, DMA_CONFIGURATION, DMA_CONFIGURATION_BURST_LENGTH(4));
}

#define TRANSMIT_ENABLE 1
#define TRANSMIT_DISABLE 2
static void vm14_gemac_hw_change_transmit(vm14_gemac_dev_t *vm14_gemac, int state)
{
	uint32_t	reg;

	reg = RREG(vm14_gemac, MAC_TRANSMIT_CONTROL);
	switch ( state ) {
		case TRANSMIT_ENABLE:
			reg |= MAC_TRANSMIT_CONTROL_TRANSMIT_ENABLE;
			break;
		case TRANSMIT_DISABLE:
			reg &= ~MAC_TRANSMIT_CONTROL_TRANSMIT_ENABLE;
			break;
	}
	WREG(vm14_gemac, MAC_TRANSMIT_CONTROL, reg);
}

static void vm14_gemac_hw_change_transmit_dma(vm14_gemac_dev_t *vm14_gemac, int state)
{
	uint32_t	reg;

	reg = RREG(vm14_gemac, DMA_CONTROL);
	switch ( state ) {
		case TRANSMIT_ENABLE:
			reg |= DMA_CONTROL_START_TRANSMIT_DMA;
			break;
		case TRANSMIT_DISABLE:
			reg &= ~DMA_CONTROL_START_TRANSMIT_DMA;
			break;
	}
	WREG(vm14_gemac, DMA_CONTROL, reg);
}

#define RECEIVE_ENABLE 1
#define RECEIVE_DISABLE 2
static void vm14_gemac_hw_change_receive(vm14_gemac_dev_t *vm14_gemac, int state)
{
	uint32_t	reg;

	reg = RREG(vm14_gemac, MAC_RECEIVE_CONTROL);
	switch ( state ) {
		case RECEIVE_ENABLE:
			reg |= (MAC_RECEIVE_CONTROL_RECEIVE_ENABLE|MAC_RECEIVE_CONTROL_STRIP_FCS|MAC_RECEIVE_CONTROL_STORE_AND_FORWARD);
			break;
		case RECEIVE_DISABLE:
			reg &= ~(MAC_RECEIVE_CONTROL_RECEIVE_ENABLE|MAC_RECEIVE_CONTROL_STRIP_FCS|MAC_RECEIVE_CONTROL_STORE_AND_FORWARD);
			break;
	}
	WREG(vm14_gemac, MAC_RECEIVE_CONTROL, reg);
}

static void vm14_gemac_hw_change_receive_dma(vm14_gemac_dev_t *vm14_gemac, int state)
{
	uint32_t	reg;

	reg = RREG(vm14_gemac, DMA_CONTROL);
	switch ( state ) {
		case RECEIVE_ENABLE:
			reg |= DMA_CONTROL_START_RECEIVE_DMA;
			break;
		case RECEIVE_DISABLE:
			reg &= ~DMA_CONTROL_START_RECEIVE_DMA;
			break;
	}
	WREG(vm14_gemac, DMA_CONTROL, reg);
}


void vm14_gemac_hw_start(vm14_gemac_dev_t *vm14_gemac)
{
	vm14_gemac_hw_change_interrupts(vm14_gemac,
			DMA_INTERRUPT_ENABLE_RECEIVE_DONE | DMA_INTERRUPT_ENABLE_TRANSMIT_DONE, IRQ_INIT);

	vm14_gemac_hw_change_transmit(vm14_gemac, TRANSMIT_ENABLE);
	vm14_gemac_hw_change_receive(vm14_gemac, RECEIVE_ENABLE);
	vm14_gemac_hw_change_transmit_dma(vm14_gemac, TRANSMIT_ENABLE);
	vm14_gemac_hw_change_receive_dma(vm14_gemac, RECEIVE_ENABLE);
}

void vm14_gemac_hw_set_tx_dma_addr(vm14_gemac_dev_t *vm14_gemac, uint32_t dma_addr)
{
	WREG(vm14_gemac, DMA_TRANSMIT_BASE_ADDRESS, dma_addr);
}

void vm14_gemac_hw_set_rx_dma_addr(vm14_gemac_dev_t *vm14_gemac, uint32_t dma_addr)
{
	WREG(vm14_gemac, DMA_RECEIVE_BASE_ADDRESS, dma_addr);
}

void vm14_gemac_hw_stop_tx(vm14_gemac_dev_t *vm14_gemac)
{
	uint32_t	reg;
	int			timeout = 20;

	vm14_gemac_hw_change_transmit_dma(vm14_gemac, TRANSMIT_DISABLE);

	/* Wait max 20 ms for transmit process to stop */
	while ( --timeout ) {
		reg = RREG(vm14_gemac, DMA_STATUS_AND_IRQ);
		if ( !DMA_STATUS_AND_IRQ_TRANSMIT_DMA_STATE(reg) )
			break;
		delay(1);
	}

	if ( !timeout )
		slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: TX DMAC failed to stop\n", __devname__);

	/* ACK Tx DMAC stop bit */
	WREG(vm14_gemac, DMA_STATUS_AND_IRQ,DMA_STATUS_AND_IRQ_TX_DMA_STOPPED);

	vm14_gemac_hw_change_interrupts(vm14_gemac, DMA_INTERRUPT_ENABLE_TRANSMIT_DONE, IRQ_DISABLE);
	vm14_gemac_hw_change_transmit(vm14_gemac, TRANSMIT_DISABLE);
}

void vm14_gemac_hw_stop_rx(vm14_gemac_dev_t *vm14_gemac)
{
	uint32_t	reg;
	int			timeout = 20;

	vm14_gemac_hw_change_interrupts(vm14_gemac, DMA_INTERRUPT_ENABLE_RECEIVE_DONE, IRQ_DISABLE);
	vm14_gemac_hw_change_receive(vm14_gemac, RECEIVE_DISABLE);
	vm14_gemac_hw_change_receive_dma(vm14_gemac, RECEIVE_DISABLE);

	/* Wait max 20 ms for receive process to stop */
	while ( --timeout ) {
		reg = RREG(vm14_gemac, DMA_STATUS_AND_IRQ);
		if ( !DMA_STATUS_AND_IRQ_RECEIVE_DMA_STATE(reg) )
			break;
		delay(1);
	}

	if ( !timeout )
		slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: RX DMAC failed to stop", __devname__);

	/* ACK the Rx DMAC stop bit */
	WREG(vm14_gemac, DMA_STATUS_AND_IRQ, DMA_STATUS_AND_IRQ_RX_DMA_STOPPED);
}

void vm14_gemac_hw_set_frame_size(vm14_gemac_dev_t *vm14_gemac, uint32_t frame_size)
{
	WREG(vm14_gemac, MAC_MAXIMUM_FRAME_SIZE, frame_size + ETH_NODATA);
	WREG(vm14_gemac, MAC_TRANSMIT_JABBER_SIZE, frame_size + ETH_NODATA * 2);
	WREG(vm14_gemac, MAC_RECEIVE_JABBER_SIZE, frame_size + ETH_NODATA * 2);
}

void vm14_gemac_hw_set_promiscuous_mode(vm14_gemac_dev_t *vm14_gemac, int set)
{
	uint32_t	reg;

	reg = RREG(vm14_gemac, MAC_ADDRESS_CONTROL);
	if ( set ) {
		reg |= MAC_ADDRESS_PROM_ENABLE;
	} else {
		reg &= ~MAC_ADDRESS_PROM_ENABLE;
	}
	WREG(vm14_gemac, MAC_ADDRESS_CONTROL, reg);
}

void vm14_gemac_hw_set_allmulticast_mode(vm14_gemac_dev_t *vm14_gemac, int set)
{
	uint32_t	hash_table[4];

	hash_table[0] = RREG(vm14_gemac, MAC_HASH_TABLE1);
	hash_table[1] = RREG(vm14_gemac, MAC_HASH_TABLE2);
	hash_table[2] = RREG(vm14_gemac, MAC_HASH_TABLE3);
	hash_table[3] = RREG(vm14_gemac, MAC_HASH_TABLE4);

	if ( set ) {
		hash_table[0] =
		hash_table[1] =
		hash_table[2] =
		hash_table[3] = 0xFFFF;
	} else {
		hash_table[0] =
		hash_table[1] =
		hash_table[2] =
		hash_table[3] = 0;
	}

	WREG(vm14_gemac, MAC_HASH_TABLE1, hash_table[0]);
	WREG(vm14_gemac, MAC_HASH_TABLE2, hash_table[1]);
	WREG(vm14_gemac, MAC_HASH_TABLE3, hash_table[2]);
	WREG(vm14_gemac, MAC_HASH_TABLE4, hash_table[3]);
}

void vm14_gemac_hw_setup_hashtable(vm14_gemac_dev_t *vm14_gemac, uint8_t *addr, int set)
{
	uint32_t	crc;
	uint32_t	hash_table[4];

	hash_table[0] = RREG(vm14_gemac, MAC_HASH_TABLE1);
	hash_table[1] = RREG(vm14_gemac, MAC_HASH_TABLE2);
	hash_table[2] = RREG(vm14_gemac, MAC_HASH_TABLE3);
	hash_table[3] = RREG(vm14_gemac, MAC_HASH_TABLE4);
	
	crc = nic_calc_crc_le(addr, ETHER_ADDR_LEN);
	crc = ENDIAN_RET32(crc);
	crc &= 0x3F;
	if ( set ) {
		hash_table[crc / 16] |= (1 << (crc % 16));
	} else {
		hash_table[crc / 16] &= ~(1 << (crc % 16));
	}

	WREG(vm14_gemac, MAC_HASH_TABLE1, hash_table[0]);
	WREG(vm14_gemac, MAC_HASH_TABLE2, hash_table[1]);
	WREG(vm14_gemac, MAC_HASH_TABLE3, hash_table[2]);
	WREG(vm14_gemac, MAC_HASH_TABLE4, hash_table[3]);
}

void vm14_gemac_hwspeed(vm14_gemac_dev_t *vm14_gemac, int speed)
{
	uint32_t	reg;

	reg = RREG(vm14_gemac, MAC_GLOBAL_CONTROL);
	
	reg &= ~MAC_GLOBAL_CONTROL_SPEED(3);
	switch ( speed ) {
		case 100:
			reg |= MAC_GLOBAL_CONTROL_SPEED(1);
			if ( vm14_gemac->gpio.pins[1].bit != 0xFF ) {
				// OUT
				vm14_gemac->gpio.base[vm14_gemac->gpio.pins[1].ddr] |= BIT(vm14_gemac->gpio.pins[1].bit);
				// SOFT
				vm14_gemac->gpio.base[vm14_gemac->gpio.pins[1].ctl] &= ~BIT(vm14_gemac->gpio.pins[1].bit);
				// UNSET
				vm14_gemac->gpio.base[vm14_gemac->gpio.pins[1].dr] &= ~BIT(vm14_gemac->gpio.pins[1].bit);
			}
			break;
		case 1000:
			reg |= MAC_GLOBAL_CONTROL_SPEED(2);
			if (  vm14_gemac->gpio.pins[1].bit != 0xFF ) {
				// OUT
				vm14_gemac->gpio.base[vm14_gemac->gpio.pins[1].ddr] |= BIT(vm14_gemac->gpio.pins[1].bit);
				// SOFT
				vm14_gemac->gpio.base[vm14_gemac->gpio.pins[1].ctl] &= ~BIT(vm14_gemac->gpio.pins[1].bit);
				// SET
				vm14_gemac->gpio.base[vm14_gemac->gpio.pins[1].dr] |= BIT(vm14_gemac->gpio.pins[1].bit);
			}
			break;
		default:
			reg |= MAC_GLOBAL_CONTROL_SPEED(0);
			break;
	}

	WREG(vm14_gemac, MAC_GLOBAL_CONTROL, reg);

	vm14_gemac_hw_speed2tx_threshold(vm14_gemac, speed);
}

void vm14_gemac_hwduplex(vm14_gemac_dev_t *vm14_gemac, int full)
{
	uint32_t	reg;

	reg = RREG(vm14_gemac, MAC_GLOBAL_CONTROL);
	
	if ( full ) {
		reg |= MAC_GLOBAL_CONTROL_FULL_DUPLEX_MODE;
	} else {
		reg &= ~MAC_GLOBAL_CONTROL_FULL_DUPLEX_MODE;
	}

	WREG(vm14_gemac, MAC_GLOBAL_CONTROL, reg);
}

static void vm14_gemac_hw_mii_wait(vm14_gemac_dev_t	*vm14_gemac)
{
	/* wait for end of transfer */
	while ( MAC_MDIO_CONTROL_WORKING(RREG(vm14_gemac, MAC_MDIO_CONTROL)) )
		delay(1);
}

uint16_t vm14_gemac_hw_mii_read(void *hdl, uint8_t phy_id, uint8_t reg_num)
{
	vm14_gemac_dev_t	*vm14_gemac = (vm14_gemac_dev_t *)hdl;
	uint32_t			val;

	vm14_gemac_hw_mii_wait(vm14_gemac);

	WREG(vm14_gemac, MAC_MDIO_CONTROL,
			    MAC_MDIO_CONTROL_READ_WRITE(1) |
			    MAC_MDIO_CONTROL_REG_ADDR(reg_num) |
			    MAC_MDIO_CONTROL_PHY_ADDR(phy_id) |
			    MAC_MDIO_CONTROL_START_FRAME(1));

	vm14_gemac_hw_mii_wait(vm14_gemac);

	val = RREG(vm14_gemac, MAC_MDIO_DATA);

	return (uint16_t)val;
}

void vm14_gemac_hw_mii_write(void *hdl, uint8_t phy_id, uint8_t reg_num, uint16_t val)
{
	vm14_gemac_dev_t	*vm14_gemac = (vm14_gemac_dev_t *)hdl;

	WREG(vm14_gemac, MAC_MDIO_DATA, val);

	vm14_gemac_hw_mii_wait(vm14_gemac);

	WREG(vm14_gemac, MAC_MDIO_CONTROL,
			    MAC_MDIO_CONTROL_START_FRAME(1) |
			    MAC_MDIO_CONTROL_PHY_ADDR(phy_id) |
			    MAC_MDIO_CONTROL_REG_ADDR(reg_num) |
			    MAC_MDIO_CONTROL_READ_WRITE(0));

	vm14_gemac_hw_mii_wait(vm14_gemac);
}


static int vm14_gemac_hw_txstat_wait(vm14_gemac_dev_t *vm14_gemac)
{
	int		cnt = 20;
	/* wait for end of transfer */
	while ( cnt-- > 0 && MAC_STATCTR_CONTROL_WORKING(RREG(vm14_gemac, MAC_TX_STATCTR_CONTROL)) )
		delay(1);

	return ( cnt >= 0 );
}

static int vm14_gemac_hw_rxstat_wait(vm14_gemac_dev_t *vm14_gemac)
{
	int		cnt = 20;
	/* wait for end of transfer */
	while ( cnt-- > 0 && MAC_STATCTR_CONTROL_WORKING(RREG(vm14_gemac, MAC_RX_STATCTR_CONTROL)) )
		delay(1);

	return ( cnt >= 0 );
}

uint32_t vm14_gemac_hw_txstat_read(vm14_gemac_dev_t *vm14_gemac, int id)
{
	uint32_t	reg;

	if ( !vm14_gemac_hw_txstat_wait(vm14_gemac) ) {
		if (vm14_gemac->cfg.verbose) {
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1,
				   "%s: TX_STAT_CTRL: 0x%08X",
						__devname__, RREG(vm14_gemac, MAC_TX_STATCTR_CONTROL));
		}
	}

	WREG(vm14_gemac, MAC_TX_STATCTR_CONTROL, (id & (MAC_STATCTR_CONTROL_START - 1)) | MAC_STATCTR_CONTROL_START);

	if ( !vm14_gemac_hw_txstat_wait(vm14_gemac) ) {
		if (vm14_gemac->cfg.verbose) {
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1,
				   "%s: TX_STAT_CTRL: 0x%08X",
						__devname__, RREG(vm14_gemac, MAC_TX_STATCTR_CONTROL));
		}
	}

	reg = (RREG(vm14_gemac, MAC_TX_STATCTR_DATA_LOW) & 0xFFFF);
	reg |= (RREG(vm14_gemac, MAC_TX_STATCTR_DATA_HIGH) & 0xFFFF) << 16;

	return reg;
}

uint32_t vm14_gemac_hw_rxstat_read(vm14_gemac_dev_t *vm14_gemac, int id)
{
	uint32_t	reg;

	if ( !vm14_gemac_hw_rxstat_wait(vm14_gemac) ) {
		if (vm14_gemac->cfg.verbose) {
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1,
				   "%s: RX_STAT_CTRL: 0x%08X",
						__devname__, RREG(vm14_gemac, MAC_RX_STATCTR_CONTROL));
		}
	}

	WREG(vm14_gemac, MAC_RX_STATCTR_CONTROL, (id & (MAC_STATCTR_CONTROL_START - 1)) | MAC_STATCTR_CONTROL_START);

	if ( !vm14_gemac_hw_rxstat_wait(vm14_gemac) ) {
		if (vm14_gemac->cfg.verbose) {
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1,
				   "%s: RX_STAT_CTRL: 0x%08X",
						__devname__, RREG(vm14_gemac, MAC_RX_STATCTR_CONTROL));
		}
	}

	reg = (RREG(vm14_gemac, MAC_RX_STATCTR_DATA_LOW) & 0xFFFF);
	reg |= (RREG(vm14_gemac, MAC_RX_STATCTR_DATA_HIGH) & 0xFFFF) << 16;

	return reg;
}

#define print_reg(reg) slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s offset 0x%x : value 0x%x", __devname__, REG1(reg), RREG(vm14_gemac, reg))
void vm14_gemac_hw_dump_registers(vm14_gemac_dev_t *vm14_gemac)
{
	slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s (%s) Logical Net %d, Reg : 0x%08x\n",
				vm14_gemac->cfg.device_description, __devname__, vm14_gemac->cfg.lan, (uint32_t)vm14_gemac->reg );
	print_reg(DMA_CONFIGURATION);
	print_reg(DMA_CONTROL);
	print_reg(DMA_STATUS_AND_IRQ);
	print_reg(DMA_INTERRUPT_ENABLE);
	print_reg(DMA_TRANSMIT_AUTO_POLL_COUNTER);
	print_reg(DMA_TRANSMIT_POLL_DEMAND);
	print_reg(DMA_RECEIVE_POLL_DEMAND);
	print_reg(DMA_TRANSMIT_BASE_ADDRESS);
	print_reg(DMA_RECEIVE_BASE_ADDRESS);
	print_reg(DMA_MISSED_FRAME_COUNTER);
	print_reg(DMA_STOP_FLUSH_COUNTER);
	print_reg(DMA_RECEIVE_INTERRUPT_MITIGATION);
	print_reg(DMA_CURRENT_TRANSMIT_DESCRIPTOR_POINTER);
	print_reg(DMA_CURRENT_TRANSMIT_BUFFER_POINTER);
	print_reg(DMA_CURRENT_RECEIVE_DESCRIPTOR_POINTER);
	print_reg(DMA_CURRENT_RECEIVE_BUFFER_POINTER);

	print_reg(MAC_GLOBAL_CONTROL);
	print_reg(MAC_TRANSMIT_CONTROL);
	print_reg(MAC_RECEIVE_CONTROL);
	print_reg(MAC_ADDRESS_CONTROL);
	print_reg(MAC_ADDRESS1_HIGH);
	print_reg(MAC_ADDRESS1_MED);
	print_reg(MAC_ADDRESS1_LOW);
	print_reg(MAC_HASH_TABLE1);
	print_reg(MAC_HASH_TABLE2);
	print_reg(MAC_HASH_TABLE3);
	print_reg(MAC_HASH_TABLE4);
	print_reg(MAC_INTERRUPT);
	print_reg(MAC_INTERRUPT_ENABLE);

	if ( vm14_gemac->mdi != NULL )
		MDI_DumpRegisters( vm14_gemac->mdi, vm14_gemac->cfg.phy_addr );
}

