/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2016 SWD Embedded Systems Ltd. All rights reserved.         *
 *                                                                           *
 * Driver for the Gigabit Ethernet network controller                        *
 * Network controller bsd media routines                                     *
 *                                                                           *
 *****************************************************************************/


#include <1892vm14_gemac.h>
#include <sys/malloc.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <device_qnx.h>


#include <sys/mman.h>


//
// this is a callback, made by the bsd media code.  We passed
// a pointer to this function during the ifmedia_init() call
// in bsd_mii_initmedia()
//
void bsd_mii_mediastatus(struct ifnet *ifp, struct ifmediareq *ifmr)
{
	vm14_gemac_dev_t *vm14_gemac = ifp->if_softc;

	vm14_gemac->bsd_mii.mii_media_active = IFM_ETHER;
	vm14_gemac->bsd_mii.mii_media_status = IFM_AVALID;

	if (vm14_gemac->force_advertise != -1) {	// link is forced

		if (vm14_gemac->cfg.flags & NIC_FLAG_LINK_DOWN) {
			vm14_gemac->bsd_mii.mii_media_active |= IFM_NONE;
			vm14_gemac->bsd_mii.mii_media_status  = 0;

		} else {	// link is up
			vm14_gemac->bsd_mii.mii_media_status |= IFM_ACTIVE;

			switch(vm14_gemac->cfg.media_rate) {
				case 0:
				vm14_gemac->bsd_mii.mii_media_active |= IFM_NONE;
				break;	
	
				case 1000*10:
				vm14_gemac->bsd_mii.mii_media_active |= IFM_10_T;
				break;
	
				case 1000*100:
				vm14_gemac->bsd_mii.mii_media_active |= IFM_100_TX;
				break;
					
				case 1000*1000:
				vm14_gemac->bsd_mii.mii_media_active |= IFM_1000_T;
				break;
	
				default:	// this shouldnt really happen, but ...
				vm14_gemac->bsd_mii.mii_media_active |= IFM_NONE;
				break;
			}
	
			if (vm14_gemac->cfg.duplex) {
				vm14_gemac->bsd_mii.mii_media_active |= IFM_FDX;
			}
		}

	} else if (!(vm14_gemac->cfg.flags & NIC_FLAG_LINK_DOWN)) {  // link is auto-detect and up

		vm14_gemac->bsd_mii.mii_media_status |= IFM_ACTIVE;

		switch(vm14_gemac->cfg.media_rate) {
			case 1000*10:
			vm14_gemac->bsd_mii.mii_media_active |= IFM_10_T;
			break;

			case 1000*100:
			vm14_gemac->bsd_mii.mii_media_active |= IFM_100_TX;
			break;
			
			case 1000*1000:
			vm14_gemac->bsd_mii.mii_media_active |= IFM_1000_T;
			break;

			default:	// this shouldnt really happen, but ...
			vm14_gemac->bsd_mii.mii_media_active |= IFM_NONE;
			break;
		}

		if (vm14_gemac->cfg.duplex) {
			vm14_gemac->bsd_mii.mii_media_active |= IFM_FDX;
		}

		// could move this to mii.c so there was no lag
		ifmedia_set(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_AUTO);

	} else {	// link is auto-detect and down
		vm14_gemac->bsd_mii.mii_media_active |= IFM_NONE;
		vm14_gemac->bsd_mii.mii_media_status = 0;

		// could move this to mii.c so there was no lag
		ifmedia_set(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_NONE);
	}

	// stuff parameter values with hoked-up bsd values
	ifmr->ifm_status = vm14_gemac->bsd_mii.mii_media_status;
	ifmr->ifm_active = vm14_gemac->bsd_mii.mii_media_active;
}


//
// this is a callback, made by the bsd media code.  We passed
// a pointer to this function during the ifmedia_init() call
// in bsd_mii_initmedia().  This function is called when
// someone makes an ioctl into us, we call into the generic
// ifmedia source, and it make this callback to actually 
// force the speed and duplex, just as if the user had
// set the cmd line options
//
int bsd_mii_mediachange(struct ifnet *ifp)
{
	vm14_gemac_dev_t	*vm14_gemac			= ifp->if_softc;
    int             old_media_rate	= vm14_gemac->cfg.media_rate;
    int             old_duplex		= vm14_gemac->cfg.duplex;
	struct ifmedia	*ifm			= &vm14_gemac->bsd_mii.mii_media;
    int             user_duplex		= ifm->ifm_media & IFM_FDX ? 1 : 0;
    int             user_media		= ifm->ifm_media & IFM_TMASK;

    if (!(ifp->if_flags & IFF_UP)) {
		if (vm14_gemac->cfg.verbose)
			slogf(_SLOGC_NETWORK, _SLOG_WARNING, "%s(): isn't up, ioctl ignored", __devname__);
	    return 0;
	}

	if (!(ifm->ifm_media & IFM_ETHER)) {
		if (vm14_gemac->cfg.verbose)
			slogf(_SLOGC_NETWORK, _SLOG_WARNING, "%s(): interface - bad media: 0x%X", 
		  __devname__, ifm->ifm_media);
		return 0;	// should never happen
	}

	switch (user_media) {
		case IFM_AUTO:		// auto-select media
			vm14_gemac->force_advertise = -1;
			vm14_gemac->cfg.media_rate	= -1;
			vm14_gemac->cfg.duplex		= -1;
			ifmedia_set(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_AUTO);
			break;

		case IFM_NONE:		// disable media
			//
			// forcing the link with a speed of zero means to disable the link
			//
			vm14_gemac->force_advertise = 0;
			vm14_gemac->cfg.media_rate	= 0; 
			vm14_gemac->cfg.duplex		= 0;
			ifmedia_set(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_NONE);
			break;

		case IFM_10_T:		// force 10baseT
			vm14_gemac->force_advertise = user_duplex ? MDI_10bTFD : MDI_10bT;
			vm14_gemac->cfg.media_rate	= 10 * 1000;
			vm14_gemac->cfg.duplex		= user_duplex;
			ifmedia_set(&vm14_gemac->bsd_mii.mii_media, 
			user_duplex ? IFM_ETHER|IFM_10_T|IFM_FDX : IFM_ETHER|IFM_10_T);
			break;

		case IFM_100_TX:	// force 100baseTX
			vm14_gemac->force_advertise = user_duplex ? MDI_100bTFD : MDI_100bT;
			vm14_gemac->cfg.media_rate	= 100 * 1000;
			vm14_gemac->cfg.duplex		= user_duplex;
			ifmedia_set(&vm14_gemac->bsd_mii.mii_media, 
			user_duplex ? IFM_ETHER|IFM_100_TX|IFM_FDX : IFM_ETHER|IFM_100_TX);
			break;

		case IFM_1000_T:	// force 1000baseT
			//
			// N.B.  I have not had good luck, trying to get gige to work half
			// duplex.  Even with different gige switches, I can only force full duplex
			//
			vm14_gemac->force_advertise = user_duplex ? MDI_1000bTFD : MDI_1000bT;
			vm14_gemac->cfg.media_rate	= 1000 * 1000;
			vm14_gemac->cfg.duplex		= user_duplex;
			ifmedia_set(&vm14_gemac->bsd_mii.mii_media, 
			user_duplex ? IFM_ETHER|IFM_1000_T|IFM_FDX : IFM_ETHER|IFM_1000_T);
			break;

		default:			// should never happen
			if (vm14_gemac->cfg.verbose)
				slogf(_SLOGC_NETWORK, _SLOG_WARNING, "%s(): - unknown media: 0x%X", __devname__, user_media);
			return 0;
			break;
	}

	// does the user want something different than it already is?
	if ((vm14_gemac->cfg.media_rate != old_media_rate)    ||
		(vm14_gemac->cfg.duplex     != old_duplex)        ||
		(vm14_gemac->cfg.flags      &  NIC_FLAG_LINK_DOWN) ) {
		// re-initialize hardware with new parameters
		ifp->if_init(ifp);

	}

	return 0;
}

void bsd_mii_initmedia(vm14_gemac_dev_t *vm14_gemac)
{
    vm14_gemac->bsd_mii.mii_ifp = &vm14_gemac->ecom.ec_if;

	ifmedia_init(&vm14_gemac->bsd_mii.mii_media, IFM_IMASK, bsd_mii_mediachange,
	  bsd_mii_mediastatus);

	// we do NOT call mii_attach() - we do our own link management

	//
	// must create these entries to make ifconfig media work
	// see net/if_media.h for defines
	//

	// ifconfig ag0 media none (x22)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_NONE, 0, NULL);

	// ifconfig ag0 media auto (x20)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_AUTO, 0, NULL);

	// ifconfig ag0 media 10baseT (x23 - half duplex)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_10_T, 0, NULL);

	// ifconfig ag0 media 10baseT-FDX (x100023)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_10_T|IFM_FDX, 0, NULL);

	// ifconfig ag0 media 100baseTX (x26 - half duplex)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_100_TX, 0, NULL);

	// ifconfig ag0 media 100baseTX-FDX (x100026 - full duplex)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_100_TX|IFM_FDX, 0, NULL);

	// ifconfig ag0 media 1000baseT (x30 - half duplex)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_1000_T, 0, NULL);

	// ifconfig ag0 media 1000baseT mediaopt fdx (x100030 - full duplex)
    ifmedia_add(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_1000_T|IFM_FDX, 0, NULL);

	// add more entries to support flow control via ifconfig media

	// link is initially down
	ifmedia_set(&vm14_gemac->bsd_mii.mii_media, IFM_ETHER|IFM_NONE);
}


