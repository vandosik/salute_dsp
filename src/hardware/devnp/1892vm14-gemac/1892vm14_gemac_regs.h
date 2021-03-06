/*****************************************************************************
*                                                                           *
* Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
*                                                                           *
* Driver for the Gigabit Ethernet network controller                        *
* Network controller register definitions                                   *
*                                                                           *
*****************************************************************************/


#ifndef VM14_GEMAC_REGS_H_INCLUDED
#define VM14_GEMAC_REGS_H_INCLUDED



/* GEMAC register offsets */

#define DMA_CONFIGURATION							REG32(0x0000)
#define DMA_CONTROL									REG32(0x0004)
#define DMA_STATUS_AND_IRQ							REG32(0x0008)
#define DMA_INTERRUPT_ENABLE						REG32(0x000C)
#define DMA_TRANSMIT_AUTO_POLL_COUNTER				REG32(0x0010)
#define DMA_TRANSMIT_POLL_DEMAND					REG32(0x0014)
#define DMA_RECEIVE_POLL_DEMAND						REG32(0x0018)
#define DMA_TRANSMIT_BASE_ADDRESS					REG32(0x001C)
#define DMA_RECEIVE_BASE_ADDRESS					REG32(0x0020)
#define DMA_MISSED_FRAME_COUNTER					REG32(0x0024)
#define DMA_STOP_FLUSH_COUNTER						REG32(0x0028)
#define DMA_RECEIVE_INTERRUPT_MITIGATION			REG32(0x002C)
#define DMA_CURRENT_TRANSMIT_DESCRIPTOR_POINTER		REG32(0x0030)
#define DMA_CURRENT_TRANSMIT_BUFFER_POINTER			REG32(0x0034)
#define DMA_CURRENT_RECEIVE_DESCRIPTOR_POINTER		REG32(0x0038)
#define DMA_CURRENT_RECEIVE_BUFFER_POINTER			REG32(0x003C)

#define MAC_GLOBAL_CONTROL							REG32(0x0100)
#define MAC_TRANSMIT_CONTROL						REG32(0x0104)
#define MAC_RECEIVE_CONTROL							REG32(0x0108)
#define MAC_MAXIMUM_FRAME_SIZE						REG32(0x010C)
#define MAC_TRANSMIT_JABBER_SIZE					REG32(0x0110)
#define MAC_RECEIVE_JABBER_SIZE						REG32(0x0114)
#define MAC_ADDRESS_CONTROL							REG32(0x0118)
#define MAC_MDIO_CLOCK_DIVISION_CONTROL				REG32(0x011C)
#define MAC_ADDRESS1_HIGH							REG32(0x0120)
#define MAC_ADDRESS1_MED							REG32(0x0124)
#define MAC_ADDRESS1_LOW							REG32(0x0128)
#define MAC_ADDRESS2_HIGH							REG32(0x012C)
#define MAC_ADDRESS2_MED							REG32(0x0130)
#define MAC_ADDRESS2_LOW							REG32(0x0134)
#define MAC_ADDRESS3_HIGH							REG32(0x0138)
#define MAC_ADDRESS3_MED							REG32(0x013C)
#define MAC_ADDRESS3_LOW							REG32(0x0140)
#define MAC_ADDRESS4_HIGH							REG32(0x0144)
#define MAC_ADDRESS4_MED							REG32(0x0148)
#define MAC_ADDRESS4_LOW							REG32(0x014C)
#define MAC_HASH_TABLE1								REG32(0x0150)
#define MAC_HASH_TABLE2								REG32(0x0154)
#define MAC_HASH_TABLE3								REG32(0x0158)
#define MAC_HASH_TABLE4								REG32(0x015C)

#define MAC_MDIO_CONTROL							REG32(0x01A0)
#define MAC_MDIO_DATA								REG32(0x01A4)
#define MAC_RX_STATCTR_CONTROL						REG32(0x01A8)
#define MAC_RX_STATCTR_DATA_HIGH					REG32(0x01AC)
#define MAC_RX_STATCTR_DATA_LOW						REG32(0x01B0)
#define MAC_TX_STATCTR_CONTROL						REG32(0x01B4)
#define MAC_TX_STATCTR_DATA_HIGH					REG32(0x01B8)
#define MAC_TX_STATCTR_DATA_LOW						REG32(0x01BC)
#define MAC_TRANSMIT_FIFO_ALMOST_FULL				REG32(0x01C0)
#define MAC_TRANSMIT_PACKET_START_THRESHOLD			REG32(0x01C4)
#define MAC_RECEIVE_PACKET_START_THRESHOLD			REG32(0x01C8)
#define MAC_TRANSMIT_FIFO_ALMOST_EMPTY_THRESHOLD	REG32(0x01CC)
#define MAC_INTERRUPT								REG32(0x01E0)
#define MAC_INTERRUPT_ENABLE						REG32(0x01E4)
#define MAC_VLAN_TPID1								REG32(0x01E8)
#define MAC_VLAN_TPID2								REG32(0x01EC)
#define MAC_VLAN_TPID3								REG32(0x01F0)

/* GEMAC register fields */

#define MAC_STATCTR_CONTROL_WORKING(VAL)			(((VAL) >> 15) & 0x1)
#define MAC_STATCTR_CONTROL_START					BIT(15)

#define STATCTR_FRAMES_TRANSMITTED_OK				0
#define STATCTR_FRAMES_TRANSMITTED_TOTAL			1
#define STATCTR_OCTETS_TRANSMITTED_OK				2
#define STATCTR_FRAMES_TRANSMITTED_ERRPR			3
#define STATCTR_FRAMES_TRANSMITTED_SINGLECLSN		4
#define STATCTR_FRAMES_TRANSMITTED_MULTIPLECLSN		5
#define STATCTR_FRAMES_TRANSMITTED_LATECLSN			6
#define STATCTR_FRAMES_TRANSMITTED_EXCESSIVECLSN	7
#define STATCTR_FRAMES_TRANSMITTED_UNICASTADDR		8
#define STATCTR_FRAMES_TRANSMITTED_MULTICASTADDR	9
#define STATCTR_FRAMES_TRANSMITTED_BROADCASTADDR	10
#define STATCTR_FRAMES_TRANSMITTED_PAUSE			11

#define STATCTR_FRAMES_RECEIVED_OK					0
#define STATCTR_FRAMES_RECEIVED_TOTAL				1
#define STATCTR_FRAMES_RECEIVED_CRCERR				2
#define STATCTR_FRAMES_RECEIVED_ALIGNERR			3
#define STATCTR_FRAMES_RECEIVED_ERROR				4
#define STATCTR_OCTETS_RECEIVED_OK					5
#define STATCTR_OCTETS_RECEIVED_TOTAL				6
#define STATCTR_FRAMES_RECEIVED_UNICASTADDR			7
#define STATCTR_FRAMES_RECEIVED_MULTICASTADDR		8
#define STATCTR_FRAMES_RECEIVED_BROADCASTADDR		9
#define STATCTR_FRAMES_RECEIVED_PAUSE				10
#define STATCTR_FRAMES_RECEIVED_LENGTHERR			11
#define STATCTR_FRAMES_RECEIVED_UNDERSIZED			12
#define STATCTR_FRAMES_RECEIVED_OVERSIZED			13
#define STATCTR_FRAMES_RECEIVED_FRAGMENTS			14
#define STATCTR_FRAMES_RECEIVED_JABBER				15
#define STATCTR_FRAMES_RECEIVED_LEN64				16
#define STATCTR_FRAMES_RECEIVED_LEN65_127			17
#define STATCTR_FRAMES_RECEIVED_LEN128_255			18
#define STATCTR_FRAMES_RECEIVED_LEN256_511			19
#define STATCTR_FRAMES_RECEIVED_LEN512_1023			20
#define STATCTR_FRAMES_RECEIVED_LEN1024_1518		21
#define STATCTR_FRAMES_RECEIVED_LEN1519PLUS			22
#define STATCTR_FRAMES_DROPPED_BUFFER_FULL			23
#define STATCTR_FRAMES_TRUNCATED_BUFFER_FULL		24

#define DMA_CONFIGURATION_SOFT_RESET				BIT(0)
#define DMA_CONFIGURATION_BURST_LENGTH(VAL)			((VAL) << 1)
#define DMA_CONFIGURATION_WAIT_FOR_DONE				BIT(16)

#define DMA_CONTROL_START_TRANSMIT_DMA				BIT(0)
#define DMA_CONTROL_START_RECEIVE_DMA				BIT(1)

#define DMA_STATUS_AND_IRQ_TRANSFER_DONE			BIT(0)
#define DMA_STATUS_AND_IRQ_TRANS_DESC_UNVAIL		BIT(1)
#define DMA_STATUS_AND_IRQ_TX_DMA_STOPPED			BIT(2)
#define DMA_STATUS_AND_IRQ_RECEIVE_DONE				BIT(4)
#define DMA_STATUS_AND_IRQ_RX_DMA_STOPPED			 BIT(6)
#define DMA_STATUS_AND_IRQ_MAC_INTERRUPT			BIT(11)
#define DMA_STATUS_AND_IRQ_TRANSMIT_DMA_STATE(VAL)	(((VAL) >> 16) & 0x7)
#define DMA_STATUS_AND_IRQ_RECEIVE_DMA_STATE(VAL)	(((VAL) >> 20) & 0xf)
// #define DMA_STATUS_AND_IRQ_TRANSMIT_DMA_STATE(VAL)	(((VAL) & 0x7000) >> 16)
// #define DMA_STATUS_AND_IRQ_RECEIVE_DMA_STATE(VAL)	(((VAL) & 0xf0000) >> 20)

#define DMA_INTERRUPT_ENABLE_TRANSMIT_DONE			BIT(0)
#define DMA_INTERRUPT_ENABLE_TRANSMIT_DESC_UNV		BIT(1)
#define DMA_INTERRUPT_ENABLE_TRANSMIT_DMA_STOP		BIT(2)
#define DMA_INTERRUPT_ENABLE_RECEIVE_DONE			BIT(4)
#define DMA_INTERRUPT_ENABLE_RECEIVE_DESC_UNV		BIT(5)
#define DMA_INTERRUPT_ENABLE_RECEIVE_DMA_STOP		BIT(6)
#define DMA_INTERRUPT_ENABLE_RECEIVE_MISS			BIT(7)
#define DMA_INTERRUPT_ENABLE_RECEIVE_MAC			BIT(8)

#define MAC_GLOBAL_CONTROL_SPEED(VAL)				((VAL) << 0)
#define MAC_GLOBAL_CONTROL_FULL_DUPLEX_MODE			BIT(2)

#define MAC_TRANSMIT_CONTROL_TRANSMIT_ENABLE		BIT(0)

#define MAC_RECEIVE_CONTROL_RECEIVE_ENABLE			BIT(0)
#define MAC_RECEIVE_CONTROL_STRIP_FCS				BIT(2)
#define MAC_RECEIVE_CONTROL_STORE_AND_FORWARD		BIT(3)

#define MAC_ADDRESS1_LOW_SIXTH_BYTE(VAL)			((VAL) << 8)
#define MAC_ADDRESS1_LOW_FIFTH_BYTE(VAL)			((VAL) << 0)
#define MAC_ADDRESS1_MED_FOURTH_BYTE(VAL)			((VAL) << 8)
#define MAC_ADDRESS1_MED_THIRD_BYTE(VAL)			((VAL) << 0)
#define MAC_ADDRESS1_HIGH_SECOND_BYTE(VAL)			((VAL) << 8)
#define MAC_ADDRESS1_HIGH_FIRST_BYTE(VAL)			((VAL) << 0)

#define MAC_ADDRESS1_ENABLE							BIT(0)
#define MAC_ADDRESS2_ENABLE							BIT(1)
#define MAC_ADDRESS3_ENABLE							BIT(2)
#define MAC_ADDRESS4_ENABLE							BIT(3)
#define MAC_ADDRESS1_INV_ENABLE						BIT(4)
#define MAC_ADDRESS2_INV_ENABLE						BIT(5)
#define MAC_ADDRESS3_INV_ENABLE						BIT(6)
#define MAC_ADDRESS4_INV_ENABLE						BIT(7)
#define MAC_ADDRESS_PROM_ENABLE						BIT(8)

#define MAC_MDIO_CONTROL_READ_WRITE(VAL)			((VAL) << 10)
#define MAC_MDIO_CONTROL_REG_ADDR(VAL)				((VAL) << 5)
#define MAC_MDIO_CONTROL_PHY_ADDR(VAL)				((VAL) << 0)
#define MAC_MDIO_CONTROL_START_FRAME(VAL)			((VAL) << 15)
#define MAC_MDIO_CONTROL_WORKING(VAL)				(((VAL) >> 15) & 0x1)

#endif
