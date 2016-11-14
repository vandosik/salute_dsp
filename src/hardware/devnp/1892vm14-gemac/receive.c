/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller receive routines                                       *
 *                                                                           *
 *****************************************************************************/
#include "1892vm14_gemac.h"
#include "bpfilter.h"

#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

void vm14_gemac_update_rx_stats(vm14_gemac_dev_t *vm14_gemac)
{
	nic_stats_t				*stats = &vm14_gemac->stats;
	nic_ethernet_stats_t	*estats = &stats->un.estats;
	struct ifnet			*ifp = &vm14_gemac->ecom.ec_if;

	ifp->if_ipackets =
	stats->rxed_ok = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_OK);
	
	ifp->if_ibytes =
	stats->octets_rxed_ok = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_OCTETS_RECEIVED_OK);
	
	stats->rxed_broadcast = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_BROADCASTADDR);
	stats->rxed_multicast = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_MULTICASTADDR);

	estats->fcs_errors = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_CRCERR);
	estats->align_errors = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_ALIGNERR);
	estats->short_packets = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_UNDERSIZED);
	estats->oversized_packets = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_OVERSIZED);
	estats->jabber_detected = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_RECEIVED_JABBER);

	estats->internal_rx_errors = vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_DROPPED_BUFFER_FULL);
	estats->internal_rx_errors += vm14_gemac_hw_rxstat_read(vm14_gemac, STATCTR_FRAMES_TRUNCATED_BUFFER_FULL);
}

void vm14_gemac_rx(vm14_gemac_dev_t *vm14_gemac, struct nw_work_thread *wtp)
{
	nic_stats_t				*gstats = &vm14_gemac->stats;
	struct ifnet			*ifp = &vm14_gemac->ecom.ec_if;
	vm14_rx_desc_t			*rdesc;
	int						cur_rx_rptr;
	off_t					phys;
	short					pkt_len;
	struct mbuf				*m, *rm;
// 	uint8_t					*dptr;

	cur_rx_rptr = vm14_gemac->cur_rx_rptr;
	rdesc = &vm14_gemac->rdesc[cur_rx_rptr];
	do {
		phys = drvr_mphys((void *)rdesc);
		if ( rdesc->status & DMA_RDES0_OWN_BIT ) {
			if (vm14_gemac->cfg.verbose > 8)
				slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: H/W owns this RxDesc, nothing Rx'd then rdesc %X",__devname__, phys);
			break;
		}
		
		if ( !(rdesc->status & DMA_RDES0_LD) ) {
			if (vm14_gemac->cfg.verbose > 8)
				slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: rdesc %X",__devname__, phys);
// 			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: rx csum %X, len %d", __devname__, RDES2_CSUM(ENDIAN_LE32(rdesc->desc2)), RDES0_PFL(ENDIAN_LE32(rdesc->desc0)));
			goto nextpkt;
		}
		
		if ( rdesc->status & (DMA_RDES0_ERR_RUNT | DMA_RDES0_ERR_CRC |
					DMA_RDES0_ERR_OVER | DMA_RDES0_ERR_JUBBER |
					DMA_RDES0_ERR_LENGTH) ) { /* There was an error. */
// 		if ( DMA_RDES0_APPST(rdesc->status) ) { /* There was an error. */
			if (vm14_gemac->cfg.verbose)
				slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: There was an error rdesc %X st %X",__devname__, phys, DMA_RDES0_APPST(rdesc->status));
		}

		pkt_len = DMA_RDES0_PKTLEN(rdesc->status);
		if (vm14_gemac->cfg.verbose > 8)
			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: pkt_len %d rdesc %X", __devname__, pkt_len, phys);

		/* Get a packet/buffer to replace the one that was filled */
		m = m_getcl_wtp(M_DONTWAIT, MT_DATA, M_PKTHDR, wtp);
		if (m == NULL) {
			if (vm14_gemac->cfg.verbose)
				slogf(_SLOGC_NETWORK, _SLOG_ERROR, "%s: Get a packet/buffer to replace the one that was filled FAILED!\n", __devname__);
			vm14_gemac->rx_discard = 1;
			ifp->if_ierrors++; // for ifconfig -v
			gstats->rx_failed_allocs++;
			if (vm14_gemac->rx_head) {
				m_freem(vm14_gemac->rx_head);
				vm14_gemac->rx_len  = 0;
				vm14_gemac->rx_head = NULL;
				vm14_gemac->rx_tail = &vm14_gemac->rx_head;
			}
			goto nextpkt;
		}
		rm = vm14_gemac->rx_mbuf[cur_rx_rptr];
		vm14_gemac->rx_mbuf[cur_rx_rptr] = m;
#ifdef VM14_DEBUG
		if ( rm == NULL )
			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: rm == NULL", __devname__);
#endif
		phys = pool_phys(m->m_data, m->m_ext.ext_page);
#ifdef USE_LIBCACHE
		CACHE_INVAL(&vm14_gemac->cachectl, m->m_data, phys, m->m_ext.ext_size);
#endif

		rdesc->buffer1 = ENDIAN_LE32(VM14_DMA_ADDR(phys));

		rm->m_pkthdr.rcvif = ifp;
		rm->m_len = pkt_len;

		*vm14_gemac->rx_tail = rm;
		vm14_gemac->rx_tail = &rm->m_next;
		vm14_gemac->rx_len += rm->m_len;

		rm = vm14_gemac->rx_head;
#ifdef VM14_DEBUG
		if ( rm == NULL )
			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: rm2 == NULL", __devname__);
#endif
		rm->m_pkthdr.len = vm14_gemac->rx_len;

		vm14_gemac->rx_len  = 0;
		vm14_gemac->rx_head = NULL;
		vm14_gemac->rx_tail = &vm14_gemac->rx_head;

#if NBPFILTER > 0
		/* Pass this up to any BPF listeners. */
		if (ifp->if_bpf)
			bpf_mtap(ifp->if_bpf, rm);
#endif

		/* stats */
// 		gstats->rxed_ok++;
// 		gstats->octets_rxed_ok += pkt_len;
// 		dptr = mtod(rm, uint8_t *);
// 		if (dptr[0] & 1) {
// 			if (K64_IS_BROADCAST(dptr))
// 				gstats->rxed_broadcast++;
// 			else
// 				gstats->rxed_multicast++;
// 		}
		/* Send it up */
		ifp->if_ipackets++;
		(*ifp->if_input)(ifp, rm);
nextpkt:
		/* Give the descriptor back to the hardware */
		rdesc->status = DMA_RDES0_OWN_BIT;
		cur_rx_rptr = (cur_rx_rptr + 1) % vm14_gemac->num_receive;
		rdesc = &vm14_gemac->rdesc[cur_rx_rptr];
		vm14_gemac->cur_rx_rptr = cur_rx_rptr;
	} while ( 1 );

	/* Kick RXDMA */
	WREG(vm14_gemac, DMA_RECEIVE_POLL_DEMAND, 1);
}
