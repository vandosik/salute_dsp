/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller DEVCTL routines                                        *
 *                                                                           *
 *****************************************************************************/


#include <1892vm14_gemac.h>
#include <net/ifdrvcom.h>
#include <sys/sockio.h>

#include <drvr/nicsupport.h>


static void set_prom_mcast(vm14_gemac_dev_t *vm14_gemac, int set)
{
	vm14_gemac_hw_set_allmulticast_mode(vm14_gemac, set);
	if ( set ) {
		vm14_gemac->ecom.ec_if.if_flags |= IFF_ALLMULTI;
	} else if (!set) {
		vm14_gemac->ecom.ec_if.if_flags &= ~IFF_ALLMULTI;
	}
}

static void set_promiscuous(vm14_gemac_dev_t *vm14_gemac, int set)
{

	vm14_gemac_hw_set_promiscuous_mode(vm14_gemac, set);
	if ( set ) {
		vm14_gemac->cfg.flags |= NIC_FLAG_PROMISCUOUS;
	} else if (!set) {
		vm14_gemac->cfg.flags &= ~NIC_FLAG_PROMISCUOUS;
	}
}

void vm14_gemac_filter(vm14_gemac_dev_t *vm14_gemac/*, int reset*/)
{
	struct ethercom			*ec;
	struct ether_multi		*enm;
	struct ether_multistep	step;
	struct ifnet			*ifp;
	int						mcentries = 0;

	ec = &vm14_gemac->ecom;
	ifp = &ec->ec_if;

	if (ifp->if_flags & IFF_PROMISC) {
		set_promiscuous(vm14_gemac, 1);
		// 	vm14_gemac_hw_set_bad_mode(vm14_gemac, 1);
		return;
	}
	// disable promiscuous
	set_promiscuous(vm14_gemac, 0);
	// clear multicast filter
	set_prom_mcast(vm14_gemac, 0);
	ETHER_FIRST_MULTI (step, ec, enm);
	while (enm != NULL) {
		if ((mcentries >= VM14_FILTER_LIMIT) || (memcmp (enm->enm_addrlo, enm->enm_addrhi,
			ETHER_ADDR_LEN)) != 0) {
			set_prom_mcast(vm14_gemac, 1);
			return;
		}
		if (vm14_gemac->cfg.verbose) {
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1,
				   "%s: enm %p %02X:%02X:%02X mcentries %d",
						__devname__, enm, enm->enm_addrlo[3],
							enm->enm_addrlo[4], enm->enm_addrlo[5], mcentries);
		}
		vm14_gemac_hw_setup_hashtable(vm14_gemac, enm->enm_addrlo, 1);
		mcentries++;
		ETHER_NEXT_MULTI (step, enm);
	}
// 	if ( mcentries ) {
// 		vm14_gemac->must_setup_address = 1;
// 	}
	if (vm14_gemac->cfg.flags & NIC_FLAG_PROMISCUOUS) {
		vm14_gemac->cfg.flags &= ~NIC_FLAG_PROMISCUOUS;
	}
// 	if (reset && mcentries && ifp->if_init != NULL)
// 		ifp->if_init(ifp);
}


int vm14_gemac_ioctl(struct ifnet * ifp, unsigned long cmd, caddr_t data)
{
	int						error = 0;
	vm14_gemac_dev_t		*vm14_gemac = ifp->if_softc;
	struct drvcom_config	*dcfgp;
	struct drvcom_stats		*dstp;
	struct ifdrv_com		*ifdc;

	switch (cmd) {
	case SIOCGDRVCOM:
		ifdc = (struct ifdrv_com *)data;
		switch (ifdc->ifdc_cmd) {
		case DRVCOM_CONFIG:
			dcfgp = (struct drvcom_config *)ifdc;

			if (ifdc->ifdc_len != sizeof(nic_config_t)) {
				error = EINVAL;
				break;
			}
			memcpy(&dcfgp->dcom_config, &vm14_gemac->cfg, sizeof(vm14_gemac->cfg));
			break;

		case DRVCOM_STATS:
			dstp = (struct drvcom_stats *)ifdc;

			if (ifdc->ifdc_len != sizeof(nic_stats_t)) {
				error = EINVAL;
				break;
			}
			vm14_gemac_update_tx_stats(vm14_gemac);
			vm14_gemac_update_rx_stats(vm14_gemac);
			memcpy(&dstp->dcom_stats, &vm14_gemac->stats, sizeof(vm14_gemac->stats));
			if (vm14_gemac->cfg.verbose) {
				vm14_gemac_hw_dump_registers(vm14_gemac);
			}
			break;

		default:
			error = ENOTTY;
		}
		break;


    case SIOCSIFMEDIA:
    case SIOCGIFMEDIA: {
		struct ifreq *ifr = (struct ifreq *)data;

        error = ifmedia_ioctl(ifp, ifr, &vm14_gemac->bsd_mii.mii_media, cmd);
        break;
		}

	default:
		error = ether_ioctl(ifp, cmd, data);
		if (error == ENETRESET) {
			/*
			 * Multicast list has changed; set the
			 * hardware filter accordingly.
			 */
			vm14_gemac_filter(vm14_gemac/*, 1*/);
			error = 0;
		}
		break;
	}

	return error;
}
