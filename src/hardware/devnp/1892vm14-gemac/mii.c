/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller MII PHY routines                                       *
 *                                                                           *
 *****************************************************************************/

#include "1892vm14_gemac.h"

void	vm14_gemac_mdi_callback (void *hdl, uint8_t phy_id, uint8_t link_state)
{
	vm14_gemac_dev_t	*vm14_gemac = (vm14_gemac_dev_t *)hdl;
	int				i, mode;
	char			*s;
	struct ifnet	*ifp = &vm14_gemac->ecom.ec_if;

	switch (link_state) {
		case MDI_LINK_UP:
			if ((i = MDI_GetActiveMedia(vm14_gemac->mdi, vm14_gemac->cfg.phy_addr, &mode)) != MDI_LINK_UP) {
				if (vm14_gemac->cfg.verbose > 8)
					slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: callback GetActiveMedia returned %x", __devname__, i);
				mode = 0;
			}

			switch (mode) {
				case MDI_10bTFD:
					s = "10 BaseT Full Duplex";
					vm14_gemac->cfg.duplex = 1;
					vm14_gemac->cfg.media_rate = 10000L;
					break;
				case MDI_10bT:
					s = "10 BaseT Half Duplex";
					vm14_gemac->cfg.duplex = 0;
					vm14_gemac->cfg.media_rate = 10000L;
					break;
				case MDI_100bTFD:
					s = "100 BaseT Full Duplex";
					vm14_gemac->cfg.duplex = 1;
					vm14_gemac->cfg.media_rate = 100000L;
					break;
				case MDI_100bT:
					s = "100 BaseT Half Duplex";
					vm14_gemac->cfg.duplex = 0;
					vm14_gemac->cfg.media_rate = 100000L;
					break;
				case MDI_100bT4:
					s = "100 BaseT4";
					vm14_gemac->cfg.duplex = 0;
					vm14_gemac->cfg.media_rate = 100000L;
					break;
				case MDI_1000bT:
					s = "1000 BaseT Half Duplex";
					vm14_gemac->cfg.duplex = 0;
					vm14_gemac->cfg.media_rate = 1000000L;
					break;
				case MDI_1000bTFD:
					s = "1000 BaseT Full Duplex";
					vm14_gemac->cfg.duplex = 1;
					vm14_gemac->cfg.media_rate = 1000000L;
					break;
				default:
					s = "Unknown";
					vm14_gemac->cfg.duplex = 0;
					vm14_gemac->cfg.media_rate = 0L;
					break;
			}
			
			if (vm14_gemac->cfg.verbose) {
				slogf (_SLOGC_NETWORK, _SLOG_INFO, "%s: Link up (%s)", __devname__, s);
			}

			vm14_gemac->cfg.flags &= ~NIC_FLAG_LINK_DOWN;
			if_link_state_change(ifp, LINK_STATE_UP);

			if (mode) {
				vm14_gemac_hwduplex(vm14_gemac, vm14_gemac->cfg.duplex);
				vm14_gemac_hwspeed(vm14_gemac, vm14_gemac->cfg.media_rate / 1000);
			}
			break;
		case MDI_LINK_DOWN:
			vm14_gemac->cfg.media_rate = vm14_gemac->cfg.duplex = -1;
	//		MDI_ResetPhy (rtl->mdi, rtl->cfg.phy_addr, WaitBusy);
	//		MDI_SyncPhy (rtl->mdi, rtl->cfg.phy_addr);
			MDI_AutoNegotiate (vm14_gemac->mdi, vm14_gemac->cfg.phy_addr, NoWait);
			vm14_gemac->cfg.flags |= NIC_FLAG_LINK_DOWN;

			if (vm14_gemac->cfg.verbose) {
				slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: Link down %d", __devname__, vm14_gemac->cfg.lan);
			}

			if_link_state_change(ifp, LINK_STATE_DOWN);
			break;
		default:
			if (vm14_gemac->cfg.verbose) {
				slogf (_SLOGC_NETWORK, _SLOG_INFO, "%s: Unknown link state %hhu", __devname__, link_state);
			}
			break;
	}
}

void vm14_gemac_mii_callout(void *arg)
{
	vm14_gemac_dev_t	*vm14_gemac   = arg;
	if ((vm14_gemac->cfg.flags & NIC_FLAG_LINK_DOWN) ||
	   (vm14_gemac->cfg.media_rate <= 0) ||
	   !vm14_gemac->pkts_received) {
		if (vm14_gemac->cfg.verbose > 12)
			slogf (_SLOGC_NETWORK, _SLOG_DEBUG1, "%s: calling MDI_MonitorPhy()", __devname__);
		MDI_MonitorPhy (vm14_gemac->mdi);
	}
	vm14_gemac->pkts_received = 0;
	callout_msec(&vm14_gemac->mii_callout, 3 * 1000, vm14_gemac_mii_callout, vm14_gemac);
}

int vm14_gemac_findphy (vm14_gemac_dev_t *vm14_gemac)

{
	int				an_capable,status;
	uint16_t		reg;
// 	struct ifnet	*ifp;

	vm14_gemac->cfg.phy_addr = vm14_gemac->phy_addr;
// 	ifp = &vm14_gemac->ecom.ec_if;

	if (vm14_gemac->mdi) {
		MDI_DeRegister ((mdi_t **)&vm14_gemac->mdi);
	}

	status = MDI_Register_Extended (vm14_gemac, vm14_gemac_hw_mii_write, vm14_gemac_hw_mii_read,
		vm14_gemac_mdi_callback, (mdi_t **)&vm14_gemac->mdi, NULL, 0, 0);
	
	if (status != MDI_SUCCESS) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: Cannot register the mii routines", __devname__);
		vm14_gemac->mdi = NULL;
		return -1;
	}
	
	callout_init(&vm14_gemac->mii_callout);

	if ( vm14_gemac->cfg.phy_addr == -1 ) {
		for (vm14_gemac->cfg.phy_addr = 0;
			vm14_gemac->cfg.phy_addr < 32; vm14_gemac->cfg.phy_addr++) {
			if (MDI_FindPhy(vm14_gemac->mdi, vm14_gemac->cfg.phy_addr) == MDI_SUCCESS)
				break;
		}

		if (vm14_gemac->cfg.phy_addr == 32) {
			slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: Cannot find an active PHY", __devname__);
			return -1;
		}
	}

	if (vm14_gemac->cfg.verbose) {
		uint32_t	phy_id;
		slogf (_SLOGC_NETWORK, _SLOG_INFO, "%s: MII transceiver found at address %d.", __devname__, vm14_gemac->cfg.phy_addr);

		phy_id = vm14_gemac_hw_mii_read(vm14_gemac, vm14_gemac->cfg.phy_addr, MDI_PHYID_1) << 16;
		phy_id |= vm14_gemac_hw_mii_read(vm14_gemac, vm14_gemac->cfg.phy_addr, MDI_PHYID_2);
		
		slogf(_SLOGC_NETWORK, _SLOG_INFO, "%s: phyid %X, vid %X, mod %X, rev %X", __devname__, phy_id, PHYID_VENDOR(phy_id), PHYID_MODEL(phy_id), PHYID_REV(phy_id));
	}

	if (MDI_InitPhy(vm14_gemac->mdi, vm14_gemac->cfg.phy_addr) != MDI_SUCCESS) {
		slogf (_SLOGC_NETWORK, _SLOG_ERROR, "%s: Cannot init the PHY status", __devname__);
		return -1;
	}


	vm14_gemac->cfg.connector = NIC_CONNECTOR_MII;
	an_capable = vm14_gemac_hw_mii_read(vm14_gemac, vm14_gemac->cfg.phy_addr, MDI_BMSR) & 8;

    //
    // if the user has specified the speed or duplex
    // or if the phy cannot auto-negotiate ...
    //
	if (vm14_gemac->force_advertise != -1 || !an_capable) {
		reg = vm14_gemac_hw_mii_read(vm14_gemac, vm14_gemac->cfg.phy_addr, MDI_BMCR);

		reg &= ~(BMCR_RESTART_AN|BMCR_SPEED_100|BMCR_FULL_DUPLEX);

		if (an_capable && vm14_gemac->force_advertise != 0) {
			/*
			 * If we force the speed, but the link partner
			 * is autonegotiating, there is a greater chance
			 * that everything will work if we advertise with
			 * the speed that we are forcing to.
			 */
			MDI_SetAdvert(vm14_gemac->mdi,
			    vm14_gemac->cfg.phy_addr, vm14_gemac->force_advertise);

			reg |= BMCR_RESTART_AN | BMCR_AN_ENABLE;

			if (vm14_gemac->cfg.verbose)
				slogf (_SLOGC_NETWORK, _SLOG_INFO, "%s: "
				    "restricted autonegotiate (%dMbps only)", __devname__,
				    vm14_gemac->cfg.media_rate/1000);
		} else {
			reg &= ~BMCR_AN_ENABLE;

			if (vm14_gemac->cfg.verbose)
				slogf (_SLOGC_NETWORK, _SLOG_INFO, "%s: forcing the link", __devname__);
		}

		if (vm14_gemac->cfg.duplex > 0)
			reg |= BMCR_FULL_DUPLEX;
		if (vm14_gemac->cfg.media_rate == 100*1000)
			reg |= BMCR_SPEED_100;

		vm14_gemac_hw_mii_write(vm14_gemac, vm14_gemac->cfg.phy_addr, MDI_BMCR, reg);

		if (reg & BMCR_AN_ENABLE)
			MDI_EnableMonitor(vm14_gemac->mdi, 1);

	} else {	// normal auto-negotiation mode

		vm14_gemac->cfg.flags |= NIC_FLAG_LINK_DOWN;
		MDI_AutoNegotiate(vm14_gemac->mdi, vm14_gemac->cfg.phy_addr, NoWait);
		status = MDI_EnableMonitor(vm14_gemac->mdi, 1);

		if (status != MDI_SUCCESS)
			slogf (_SLOGC_NETWORK, _SLOG_ERROR,
			    "%s: MDI_EnableMonitor returned %x", __devname__, status);
	}
	
	return (0);
}
