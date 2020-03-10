

#ifndef _MCOM_DSP
#define _MCOM_DSP

#include <elcore-manager.h>

#define DLCR30M_MAX_CORES					2

#define DLCR30M_IRQ_NUM						0x21

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

//size of one block
#define DLCR30M_BANK_SIZE					0x8000

//------------------ Delcore common regs --------------------------------//


#define DLCR30M_MASKR							DLCR30M_CMN_REGS	//R/W

#define DLCR30M_QSTR							(DLCR30M_CMN_REGS + 4)	//R
	#define DLCR30M_QSTR_CORE_MASK(core_num)		(0xF << (8 * (core_num)))
	#define DLCR30M_QSTR_MASK						(DLCR30M_QSTR_CORE_MASK(0) | DLCR30M_QSTR_CORE_MASK(1))
	#define DLCR30M_QSTR_PI(core_num)				(1<<(8 * core_num))
	#define DLCR30M_QSTR_SE(core_num)				(1<<(1 + 8 * core_num))
	#define DLCR30M_QSTR_BREAK(core_num)			(1<<(2 + 8 * core_num))
	#define DLCR30M_QSTR_STP(core_num)				(1<<(3 + 8 * core_num))
	#define DLCR30M_QSTR_dBREAK(core_num)			(1<<(4 + 8 * core_num))
	#define DLCR30M_QSTR_WAIT						(1<<5)
	#define DLCR30M_QSTR_INT_MEM_ERR 				(1<<6)

#define DLCR30M_INTR_PI_0						0
#define DLCR30M_INTR_SE_0						1
#define DLCR30M_INTR_BREAK_0					2
#define DLCR30M_INTR_STP_0						3
#define DLCR30M_INTR_PI_1						8
#define DLCR30M_INTR_SE_1						9
#define DLCR30M_INTR_BREAK_1					10
#define DLCR30M_INTR_STP_1						11
#define DLCR30M_INTR_WAIT						28

#define DLCR30M_CSR								(DLCR30M_CMN_REGS + 8)	//R/W
	#define DLCR30M_CSR_SYNC_START					(1 << 0)
	#define DLCR30M_CSR_SYNC_WORK					(1 << 1)
	#define DLCR30M_CSR_PM_CONFIG(val)				((val & 0x3) << 2)
	#define DLCR30M_CSR_PM_CONFIG_MASK				(0x3 << 2)
		#define DLCR30M_PMCONF_1					0
		#define DLCR30M_PMCONF_3					2
		#define DLCR30M_PMCONF_4					3
	#define DLCR30M_CSR_HEN							(1 << 16)
	#define DLCR30M_CSR_DEN							(1 << 17)
	#define DLCR30M_CSR_LEN							(1 << 18)
	#define DLCR30M_CSR_DPTR(val)					((val & 0x3) << 20)
	#define DLCR30M_CSR_LIMIT(val)					((val & 0x3F) << 24)

#define DLCR30M_TOTAL_RUN						(DLCR30M_CMN_REGS + 12)	//R/W

#define DLCR30M_TOTAL_CLK						(DLCR30M_CMN_REGS + 16)	//R/W

//------------------ elcore core regs --------------------------------//
#define DLCR30M_DSCR	  						0x100			//R/W
	#define DLCR30M_DSCR_PI							(1 << 0)
	#define DLCR30M_DSCR_SE							(1 << 1)
	#define DLCR30M_DSCR_BRK						(1 << 2)
	#define DLCR30M_DSCR_STP						(1 << 3)
	#define DLCR30M_DSCR_WT							(1 << 4)
	#define DLCR30M_DSCR_RUN						(1 << 14)
#define DLCR30M_SR								0x104			//R/W 16
#define DLCR30M_IDR								0x108			//R   16
#define DLCR30M_EFR								0x10C			//R   32
#define DLCR30M_DSTART							0x10C			//W   32
#define DLCR30M_IRQR							0x110			//R/W 32
#define DLCR30M_IMASKR							0x114			//R/W 32
#define DLCR30M_TMR								0x118			//R/W 32
#define DLCR30M_ARBR							0x11C			//R/W 16
#define DLCR30M_PC								0x120			//R/W 16
#define DLCR30M_SS								0x124			//R/W 16
#define DLCR30M_LA								0x128			//R/W 16
#define DLCR30M_CSL								0x12C			//R/W 16
#define DLCR30M_LC								0x130			//R/W 16
#define DLCR30M_CSH								0x134			//R/W 16
#define DLCR30M_SP								0x138			//R/W 16
#define DLCR30M_SAR0							0x13C			//R/W 16
#define DLCR30M_CNTR							0x140			//R/W 16
#define DLCR30M_SAR(num)						(num * 4 + 0x144)//R/W 16

#define DLCR30M_CCR								0x160			//R/W 16
#define DLCR30M_PDNR							0x164			//R/W 16
#define DLCR30M_SFR								0x168			//R/W 32
#define DLCR30M_QMASKR(num)						(num * 4 + 0x170)//R/W 32

#define DLCR30M_A(index)						(index * 4 + 0x80)//R/W 32
#define DLCR30M_I(index)						(index * 4 + 0xA0)//R/W 32/16??????
#define DLCR30M_M(index)						(index * 8 + 0xC0)//R/W 32/16?????
#define DLCR30M_AT								0xE0				//R/W 32
#define DLCR30M_IT								0xE4				//R/W 16
#define DLCR30M_MT								0xE8				//R/W 16
#define DLCR30M_DT								0xEC				//R/W 16
#define DLCR30M_IVAR							0xFC				//R/W 16

#define DLCR30M_R2L(index)						(index * 2)		 //R/W? 32    0,2,4...30
#define DLCR30M_R1L(index)						(0x40 + (index * 2 - 2))//R/W? 32   1,3,5...31  
#define DLCR30M_AC(index)						(index * 4 + 0x200)
// #define DLCR30M_PB_ERR_SR 0x340				//R/W? 32

#define DLCR30M_dbDCSR							0x500			//R/W 16
#define DLCR30M_SMASKR							0x514			//R/W 32
#define DLCR30M_Cnt_RUN							0x518			//R/W 32
#define DLCR30M_dbPCa							0x524			//R 16
#define DLCR30M_dbPCf							0x528			//R 16
#define DLCR30M_dbPCd							0x52C			//R 16
#define DLCR30M_dbPCe							0x520			//R 16
#define DLCR30M_dbPCe1							0x530			//R 16
#define DLCR30M_dbPCe2							0x534			//R 16
#define DLCR30M_dbPCe3							0x538			//R 16
#define DLCR30M_dbSAR0							0x53C			//R/W 16
#define DLCR30M_dbCNTR							0x540			//R/W 16
#define DLCR30M_dbSAR(index)					(index * 4 + 0x544) //R/W 16


#define dsp_get_reg16(CORE_VAR,REG_NAME)						*((uint16_t*)((CORE_VAR)->regs + REG_NAME))
#define dsp_get_reg32(CORE_VAR,REG_NAME)						*((uint32_t*)((CORE_VAR)->regs + REG_NAME))
#define dsp_get_reg64(CORE_VAR,REG_NAME)						*((uint64_t*)((CORE_VAR)->regs + REG_NAME))
#define dsp_set_reg16(CORE_VAR,REG_NAME,VALUE)					*((uint16_t*)((CORE_VAR)->regs + REG_NAME)) = VALUE;
#define dsp_set_reg32(CORE_VAR,REG_NAME,VALUE)					*((uint32_t*)((CORE_VAR)->regs + REG_NAME)) = VALUE;
#define dsp_set_reg64(CORE_VAR,REG_NAME,VALUE)					*((uint64_t*)((char*)(CORE_VAR)->regs + \
																	(uint64_t)(REG_NAME))) = (uint64_t)(VALUE);
																	
#define dsp_set_bit_reg16(CORE_VAR,REG_NAME,BIT_NUM)			dsp_set_reg16(CORE_VAR,REG_NAME, \
														dsp_get_reg16(CORE_VAR,REG_NAME) | (1<<BIT_NUM))

#define dsp_set_bit_reg32(CORE_VAR,REG_NAME,BIT_NUM)			dsp_set_reg32(CORE_VAR,REG_NAME, \
														dsp_get_reg32(CORE_VAR,REG_NAME) | (1<<BIT_NUM))

#define dsp_set_bit_reg64(CORE_VAR,REG_NAME,BIT_NUM)			dsp_set_reg64(CORE_VAR,REG_NAME, \
														dsp_get_reg64(CORE_VAR,REG_NAME) | (1<<BIT_NUM))

#define dsp_clr_bit_reg64(CORE_VAR,REG_NAME,BIT_NUM)			dsp_set_reg64(CORE_VAR,REG_NAME, \
														dsp_get_reg64(CORE_VAR,REG_NAME) & ~(1<<BIT_NUM))

#define dsp_clr_bit_reg64(CORE_VAR,REG_NAME,BIT_NUM)			dsp_set_reg64(CORE_VAR,REG_NAME, \
														dsp_get_reg64(CORE_VAR,REG_NAME) & ~(1<<BIT_NUM))

#define dsp_clr_bit_reg64(CORE_VAR,REG_NAME,BIT_NUM)			dsp_set_reg64(CORE_VAR,REG_NAME, \
														dsp_get_reg64(CORE_VAR,REG_NAME) & ~(1<<BIT_NUM))

#define addr2delcore30m(addr)					((addr & 0xFFFFF) >> 2)

typedef struct delcore30m_firmware {
	uint32_t		cores;
	size_t			size;
	uint8_t			*data;
} delcore30m_firmware;

#define DLCR30M_FWREADY							1
#define DLCR30M_FWEMPTY							0

#include <sdma.h>

typedef struct {
	struct delcore30m_t*	cluster;//указател на структуру кластера
	uint8_t*				xyram;
	uint32_t				xyram_phys;
	uint8_t*				pram;
	uint32_t				pram_phys;
// 	uint8_t*	stack;
	uint32_t				fw_size;
	uint8_t					fw_ready;
	uint8_t*				regs;		//core regs
	uint8_t					id; //DLCR30M_IDR (0x108) 
	uint32_t				job_id; //job on DSP core
} dsp_core;

typedef struct {
	ELCORE_DEV	  		drvhdl;
	uint8_t*			base;
	uint32_t			pbase;
	uint8_t*			regs;		//cmn regs
	uint8_t				pm_conf;
	dsp_core			core[DLCR30M_MAX_CORES];
	struct sdma_channel	sdma[SDMA_MAX_CHANNELS];
// 	uint8_t* nbsr_sic;
	uint32_t			core_count;
	int					dma_count;
	uint32_t			irq;
	int					irq_hdl;
} delcore30m_t;


extern void*	elcore_func_init(void *hdl, char *options);
extern void		elcore_func_fini(void *hdl);
//must not be firmware structure, or define it at public 
extern int		elcore_set_pram(void *hdl, void *frmwr);  //copy data to pram of target core, must
extern int		elcore_release_pram(void *hdl, uint32_t core_num); //realease pram of
extern int		elcore_reset_core(void *hdl, uint32_t core_num);
extern int		elcore_start_core(void *hdl, uint32_t core_num);
extern int		elcore_stop_core(void *hdl, uint32_t core_num);
extern int		elcore_core_read(void *hdl, /*void *data, void* offset*/uint32_t core_num, void* to, void* offset, 
uint32_t size); //use firmware struct as data
extern int		elcore_core_write(void *hdl, /*void *data, void* offset*/uint32_t core_num, void* from, void* offset, 
uint32_t size);
extern int dsp_cluster_print_regs(void *hdl);
extern int elcore_ctl(void *hdl, int cmd, void *msg, int msglen, int *nbytes, int *info );
extern int elcore_interrupt_thread(void *hdl);
extern int elcore_dmarecv( void *hdl, uint32_t core_num, uint32_t to,  uint32_t offset, uint32_t size);
extern int elcore_dmasend( void *hdl, uint32_t core_num, uint32_t from, uint32_t offset, uint32_t size);

// extern int elcore_job_status(void *hdl, uint32_t job_block); 

elcore_funcs_t elcore_funcs = {
	sizeof(elcore_funcs_t),
	elcore_func_init,          /* init() */
	elcore_func_fini,             /* fini() */
	elcore_core_write,
	elcore_core_read,
	elcore_start_core,
	elcore_stop_core,
//     elcore_job_status,
	dsp_cluster_print_regs,
	elcore_ctl,
	elcore_interrupt_thread,
	elcore_dmasend,
	elcore_dmarecv
};
#endif
