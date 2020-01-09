

#ifndef _MCOM_DSP
#define _MCOM_DSP

#include <elcore-manager.h>

#define DLCR30M_MAX_CORES				2

#define DLCR30M_BASE						0x37000000UL
#define DLCR30M_END							0x3A87FFFCUL
#define DLCR30M_SIZE						(DLCR30M_END - DLCR30M_BASE)

#define DLCR30M_CMN_REGS_PHYS				0x37001000UL
#define DLCR30M_CMN_REGS					(DLCR30M_CMN_REGS_PHYS - DLCR30M_BASE)
#define DLCR30M_CMN_REGS_SIZE				0x000001C



#define DLCR30M_XBUF_PHYS					0x37001100UL
#define DLCR30M_XBUF						(DLCR30M_XBUF_PHYS - DLCR30M_BASE)
#define DLCR30M_XBUF_SIZE					256

#define DLCR30M_JPEG_ENC_PHYS				0x37001200UL
#define DLCR30M_JPEG_ENC					(DLCR30M_JPEG_ENC_PHYS - DLCR30M_BASE)

#define DLCR30M_BUF0_JPEG_ENC_PHYS			0x3A480000UL
#define DLCR30M_BUF0_JPEG_ENC				(DLCR30M_BUF0_JPEG_ENC_PHYS - DLCR30M_BASE)

#define DLCR30M_BUF1_JPEG_ENC_PHYS			0x3A488000UL
#define DLCR30M_BUF1_JPEG_ENC				(DLCR30M_BUF1_JPEG_ENC_PHYS - DLCR30M_BASE)

#define DLCR30M_DSP0_REGS_PHYS				0x37000000UL
#define DLCR30M_DSP0_REGS					(DLCR30M_DSP0_REGS_PHYS - DLCR30M_BASE)
#define DLCR30M_DSP0_REGS_SIZE				0x00001000

#define DLCR30M_DSP1_REGS_PHYS				0x37010000UL
#define DLCR30M_DSP1_REGS					(DLCR30M_DSP1_REGS_PHYS - DLCR30M_BASE)
#define DLCR30M_DSP1_REGS_SIZE				0x00001000

//four blocks of 32Kbytes 0x0 - 0x7fff to dsp (A0-A7,AT regs)
#define DLCR30M_DSP0_XYRAM_PHYS				0x3A400000UL
#define DLCR30M_DSP0_XYRAM					(DLCR30M_DSP0_XYRAM_PHYS - DLCR30M_BASE)
#define DLCR30M_DSP0_XYRAM_SIZE				0x00020000

//four blocks of 32Kbytes 0x8000 - 0xffff to dsp (A0-A7,AT regs)
#define DLCR30M_DSP1_XYRAM_PHYS				0x3A420000UL
#define DLCR30M_DSP1_XYRAM					(DLCR30M_DSP1_XYRAM_PHYS - DLCR30M_BASE)
#define DLCR30M_DSP1_XYRAM_SIZE				0x00020000

//one block of 32Kbytes  0x0 - 0x1fff to dsp0 (PC reg)
#define DLCR30M_DSP0_PRAM_PHYS				0x3A600000UL
#define DLCR30M_DSP0_PRAM					(DLCR30M_DSP0_PRAM_PHYS - DLCR30M_BASE)
#define DLCR30M_DSP0_PRAM_SIZE				0x00008000

//one block of 32Kbytes  0x0 - 0x1fff to dsp1 (PC reg)
#define DLCR30M_DSP1_PRAM_PHYS				0x3A620000UL
#define DLCR30M_DSP1_PRAM					(DLCR30M_DSP1_PRAM_PHYS - DLCR30M_BASE)
#define DLCR30M_DSP1_PRAM_SIZE				0x00008000

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
