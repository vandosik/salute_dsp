

#ifndef _MCOM_DSP
#define _MCOM_DSP

#include <elcore-manager.h>

#define DLCR30M_MAX_CORES				2

#define DLCR30M_BASE						0x37000000UL
#define DLCR30M_END							0x3A87FFFCUL
#define DLCR30M_SIZE						(DLCR30M_END - DLCR30M_BASE)

#define DLCR30M_CMN_REGS					(0x37001000 - DLCR30M_BASE)
#define DLCR30M_CMN_REGS_SIZE				(0x37001018 - DLCR30M_CMN_REGS)

#define DLCR30M_XBUF						(0x37001100 - DLCR30M_BASE)
#define DLCR30M_XBUF_SIZE					(0x370011FC - DLCR30M_XBUF)

#define DLCR

#define 

typedef struct {
	struct dsp_cluster* cluster;//указател на структуру кластера
	uint8_t*	xyram;
	uint8_t*	pram;
// 	uint8_t*	stack;
	uint8_t		firmware_size;
	uint8_t*	dsp_regs;
	uint8_t		id;
} dsp_core;

typedef struct {
	ELCORE_DEV      drvhdl;
	uint8_t* base;
	uint8_t* cmn_regs;
	dsp_core core[DELCORE30M_MAX_CORES];
	//         dma_channel_t dma[8];
// 	uint8_t* nbsr_sic;
	int core_count;
	int dma_count;
} delcore30m_t;


extern void *elcore_func_init(void *hdl, char *options);
extern void elcore_func_fini(void *hdl);


elcore_funcs_t elcore_funcs = {
	sizeof(elcore_funcs_t),
	elcore_func_init,          /* init() */
	elcore_func_fini             /* fini() */
};
#endif
