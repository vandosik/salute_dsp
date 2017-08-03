/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller transmit routines                                      *
 *                                                                           *
 *****************************************************************************/
#include "1892vm14_gemac.h"
#include "bpfilter.h"

#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <net/if_vlanvar.h>
// #define VM14_DEBUG 2
//
// this function is called only if the packet is ridiculously
// fragmented, and will fit into one cluster
//
static struct mbuf *vm14_gemac_defrag(struct mbuf *m)
{
	struct mbuf *m2;

	if (m->m_pkthdr.len > MCLBYTES) {
		m_freem(m);
		return NULL;
	}

	MGET(m2, M_DONTWAIT, MT_DATA);
	if (m2 == NULL) {
		m_freem(m);
		return NULL;
	}

	M_COPY_PKTHDR(m2, m);

	MCLGET(m2, M_DONTWAIT);
	if ((m2->m_flags & M_EXT) == 0) {
		m_freem(m);
		m_freem(m2);
		return NULL;
	}

	m_copydata(m, 0, m->m_pkthdr.len, mtod(m2, caddr_t));
	m2->m_pkthdr.len = m2->m_len = m->m_pkthdr.len;

	m_freem(m);

	return m2;
}

void vm14_gemac_update_tx_stats(vm14_gemac_dev_t *vm14_gemac)
{
	nic_stats_t				*stats = &vm14_gemac->stats;
	nic_ethernet_stats_t	*estats = &stats->un.estats;
	struct ifnet			*ifp = &vm14_gemac->ecom.ec_if;
	
	ifp->if_opackets =
	stats->txed_ok = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_OK);
	
	ifp->if_obytes =
	stats->octets_txed_ok = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_OCTETS_TRANSMITTED_OK);
	
	stats->txed_broadcast = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_BROADCASTADDR);
	stats->txed_multicast = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_MULTICASTADDR);

	estats->internal_tx_errors = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_ERRPR);
	estats->single_collisions = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_SINGLECLSN);
	estats->multi_collisions= vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_MULTIPLECLSN);
	estats->late_collisions = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_LATECLSN);
	estats->xcoll_aborted = vm14_gemac_hw_txstat_read(vm14_gemac, STATCTR_FRAMES_TRANSMITTED_EXCESSIVECLSN);
}

void vm14_gemac_reap(vm14_gemac_dev_t *vm14_gemac)
{
	int				cur_tx_rptr = vm14_gemac->cur_tx_rptr;
	struct ifnet	*ifp = &vm14_gemac->ecom.ec_if;
	struct mbuf		*m;
	int				i, to_reap = 0, cnt = 0;
	vm14_tx_desc_t	*desc;

	if ( vm14_gemac->tx_free == vm14_gemac->num_transmit ) {
		return;
	}
	for ( i = 0, cnt = cur_tx_rptr; ; i++ ) {
		desc = &vm14_gemac->tdesc[cnt];
		if ( (desc->status & DMA_TDES0_OWN_BIT) ) {
			break;
		}
		if ( desc->misc & DMA_TDES1_LS )
			to_reap++;
		cnt = (cnt + 1) % vm14_gemac->num_transmit;
		if ( cnt == vm14_gemac->cur_tx_wptr ) {
			break;
		}
	}
		
	if ( !to_reap ) {
		return;
	}
	cnt = 0;
	while ( to_reap ) {
		desc = &vm14_gemac->tdesc [cur_tx_rptr];
		if ( (m = vm14_gemac->tx_mbuf[cur_tx_rptr]) != NULL ) {
			m_freem(m);
			vm14_gemac->tx_mbuf[cur_tx_rptr] = NULL;
		}
		if ( desc->misc & DMA_TDES1_LS ) {
			if (!(desc->status & DMA_TDES0_STATUS)) {
				ifp->if_opackets++;
			} else if ( vm14_gemac->cfg.verbose ) {
				slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: transmit[%d] info %08X, misc %08X, len %d", __devname__, cur_tx_rptr, (desc->status & DMA_TDES0_STATUS), desc->misc, (desc->misc & VM14_MAXBUFSIZE));
			}
			to_reap--;
			desc->misc &= ~DMA_TDES1_LS;
		}
		cur_tx_rptr = (cur_tx_rptr + 1) % vm14_gemac->num_transmit;
		vm14_gemac->tx_free++;
		/* This is to avoid the reap routine looping for too long */
		if ( ++cnt == vm14_gemac->tx_reap ) {
			break;
		}
	}

	vm14_gemac->cur_tx_rptr = cur_tx_rptr;
}

void vm14_gemac_start(struct ifnet *ifp)
{
	vm14_gemac_dev_t		*vm14_gemac;
	struct mbuf				*m, *m0;
	struct nw_work_thread	*wtp;
	vm14_tx_desc_t		*tdesc = NULL;
	int						cur_tx_wptr;
	int						free_wptr;
	int						num_frag;
	int						nmbuf;
	off_t					phys;

	vm14_gemac = ifp->if_softc;
	wtp = WTP;

	if ((ifp->if_flags_tx & IFF_RUNNING) == 0 || vm14_gemac->start_running || (vm14_gemac->cfg.flags & NIC_FLAG_LINK_DOWN)) {
		NW_SIGUNLOCK_P(&ifp->if_snd_ex, vm14_gemac->iopkt, wtp);
		if ( vm14_gemac->cfg.verbose )
			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: early transmit", __devname__);
		return;
	}

	vm14_gemac->start_running = 1;
#if VM14_DEBUG > 1
	slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: start", __devname__);
#endif

	if (vm14_gemac->tx_free < vm14_gemac->num_transmit / 4) {
#if VM14_DEBUG > 0
		if ( vm14_gemac->cfg.verbose > 12 )
			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: reap", __devname__);
#endif
		vm14_gemac_reap(vm14_gemac);
		if (vm14_gemac->tx_free < vm14_gemac->num_transmit / 4) {
			ifp->if_flags_tx |= IFF_OACTIVE;
// 			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: not enough tx descriptors, try later", __devname__);
			goto done; // not enough tx descriptors, try later
		}
	}
	for (;;) {
		IFQ_POLL(&ifp->if_snd, m0);
		if (m0 == NULL) {
#if VM14_DEBUG > 0
			if ( vm14_gemac->cfg.verbose > 12 )
				slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: IFQ_POLL 0", __devname__);
#endif
			break;
		}

		if (!vm14_gemac->tx_free) {
#if VM14_DEBUG > 0
			if ( vm14_gemac->cfg.verbose > 1 )
				slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: tx_free == 0", __devname__);
#endif
			ifp->if_flags_tx |= IFF_OACTIVE;
			vm14_gemac->stats.tx_failed_allocs++;
			break;
		}
			
			

		/*
		 * Can look at m to see if you have the resources
		 * to transmit it.
		 */
		IFQ_DEQUEUE(&ifp->if_snd, m0);
		// count up mbuf fragments
		for (num_frag=0, m=m0; m; num_frag++) {
#if VM14_DEBUG > 0
			if (vm14_gemac->cfg.verbose > 12)
				slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: num_frag %d m->len %d", __devname__, num_frag, m->m_len);
#endif
			m = m->m_next;
		}
		// ridiculously fragmented?
		if (num_frag > vm14_gemac->tx_free/*min(vm14_gemac->tx_reap, vm14_gemac->tx_free)*/) {
			if ( vm14_gemac->cfg.verbose > 1 )
				slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: num_frag %d - ridiculously fragmented", __devname__, num_frag);
			//
			// This should very rarely (hopefully never) happen
			//
			// Is this a huge payload?
			//
			if (m0->m_pkthdr.len > MCLBYTES) {
				//
				// Could be TSO or jumbo.  Either way, we dont call defrag
				// So, will it actually fit into the descriptor ring, as is?
				//
				// Make sure there are as many free as possible before we check
				//
				vm14_gemac_reap(vm14_gemac);

				if ( vm14_gemac->tx_free > num_frag ) {
					// we got lucky - it will fit into the tx descr ring
					if ( vm14_gemac->cfg.verbose > 1 ) {
						slogf (_SLOGC_NETWORK, _SLOG_WARNING, "%s: warning: heavily fragmented large packet transmitted: "
							"size %d, num_frag %d, free tx descr %d", 
							__devname__, m0->m_pkthdr.len, num_frag, vm14_gemac->tx_free);
					}
				} else {
					if ( vm14_gemac->cfg.verbose ) {
						slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: dropped heavily fragmented huge packet: "
						"size %d, num_frag %d, free tx descr %d", 
						__devname__, m0->m_pkthdr.len, num_frag, vm14_gemac->tx_free);
					}
					m_freem(m0);
					vm14_gemac->stats.tx_failed_allocs++;
					ifp->if_oerrors++;
					goto done;
				}
			} else {
				// 
				// its safe to call the defrag routine - we know
				// the entire payload will fit into a cluster
				//
				if ( (m = vm14_gemac_defrag(m0)) == NULL) {
					if (vm14_gemac->cfg.verbose)
						slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: vm14_gemac_defrag() failed", __devname__);
					vm14_gemac->stats.tx_failed_allocs++;
					ifp->if_oerrors++;
					goto done;
				} else if ( vm14_gemac->cfg.verbose > 1 ) {
					slogf (_SLOGC_NETWORK, _SLOG_WARNING, "%s: warning: heavily fragmented normal packet transmitted: "
						"defrag worked: size %d, orignal num_frag %d", 
						__devname__, m->m_pkthdr.len, num_frag);
				}
				m0 = m;
			
				// must re-count mbuf fragments again
				for (num_frag=0, m=m0; m; num_frag++) {
					m = m->m_next;
				}
				if ( vm14_gemac->tx_free < num_frag ) {
					if ( vm14_gemac->cfg.verbose ) {
						slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: dropped heavily fragmented packet: "
						"size %d, num_frag %d, free tx descr %d", 
						__devname__, m0->m_pkthdr.len, num_frag, vm14_gemac->tx_free);
					}
					m_freem(m0);
					vm14_gemac->stats.tx_failed_allocs++;
					ifp->if_oerrors++;
					goto done;
				}
			}
		}
		
		// 
		// we know that we have room in the tx descriptor ring
		// for this transmission, so run through all the linked
		// mbufs, flushing the cpu cache and loading the descriptors 
		// 
		cur_tx_wptr = vm14_gemac->cur_tx_wptr;
		free_wptr = cur_tx_wptr;
		nmbuf = 0;
		for (m = m0; m; m = m->m_next) {
			if (!m->m_len) {
				// harmless except when hw csum is turned on
				num_frag--;  // must adjust for free descr count below
				continue;
			}
			tdesc = &vm14_gemac->tdesc[cur_tx_wptr];
			if ( vm14_gemac->cfg.verbose && (tdesc->status & DMA_TDES0_OWN_BIT) ) {
				slogf(_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: tdesc->next %x OWN", __devname__, tdesc->buffer2);
			}
			phys = mbuf_phys(m);
#ifdef USE_LIBCACHE
			CACHE_FLUSH(&vm14_gemac->cachectl, m->m_data, phys, m->m_len);
#endif
// 			tdesc->status = 0;
			tdesc->buffer1 = VM14_DMA_ADDR(phys);
// 			tdesc->misc &= VM14_MAXBUFSIZE;
			tdesc->misc = DMA_TDES1_SAC | VM14_BUFSIZE(m->m_len);
#if VM14_DEBUG > 1
			slogf(_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: tdesc->next %p", __devname__, tdesc->buffer2);
#endif
			if (!nmbuf) {
				tdesc->misc |= DMA_TDES1_FS;
			} else {
				tdesc->status = DMA_TDES0_OWN_BIT;
			}
// 			slogf(_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: [%d] tdesc->buffer1 %08X tdesc->misc %08X", __devname__, cur_tx_wptr, tdesc->buffer1, tdesc->misc);
			free_wptr = cur_tx_wptr;
			cur_tx_wptr = (cur_tx_wptr + 1) % vm14_gemac->num_transmit;
			nmbuf++;
		}
#if VM14_DEBUG > 1
		slogf(_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: free_wptr %d tx_free %d", __devname__, free_wptr, vm14_gemac->tx_free - num_frag);
#endif
		
		/* Store a pointer to the packet for freeing later */
		vm14_gemac->tx_mbuf[free_wptr] = m0;

		vm14_gemac->tx_free -= num_frag;
		if ( vm14_gemac->tx_free < MIN_NUM_TX/*vm14_gemac->num_transmit / 4*/ ) {
			tdesc->misc |= DMA_TDES1_IOC;
		}
		tdesc->misc |= DMA_TDES1_LS;

		tdesc = &vm14_gemac->tdesc[vm14_gemac->cur_tx_wptr];
		tdesc->status = DMA_TDES0_OWN_BIT;
		
		vm14_gemac->cur_tx_wptr = cur_tx_wptr;
		/* kick the DMA */
		WREG(vm14_gemac, DMA_TRANSMIT_POLL_DEMAND, 1);

	
#if NBPFILTER > 0
		/* Pass the packet to any BPF listeners. */
#if VM14_DEBUG > 1
		slogf(_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: Pass the packet to any BPF listeners. (%p)", __devname__, ifp->if_bpf);
#endif
		if (ifp->if_bpf) {
			bpf_mtap(ifp->if_bpf, m0);
		}
#endif

	} // for

done:
	vm14_gemac->start_running = 0;
	ifp->if_flags_tx &= ~IFF_OACTIVE;
#if VM14_DEBUG > 1
	slogf(_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: done", __devname__);
#endif
	NW_SIGUNLOCK_P(&ifp->if_snd_ex, vm14_gemac->iopkt, wtp);
}
