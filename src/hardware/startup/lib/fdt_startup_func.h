#ifndef _STARTUP_FDT_H
#define _STARTUP_FDT_H

#include <sys/types.h>

#define USE_FDT_CPU_PLL					0x01
#define USE_FDT_MEM_CONFIG				0x02
#define USE_FDT_SDMMC_CONFIG			0x04

extern uint8_t                      fdt_flags;
extern paddr32_t                    fdt_addr;
extern unsigned                     fdt_size;

/*
 * process_fdt - set up fdt address and fdt_flags ( what to take from fdt: memory, clk )
 */
int process_fdt(int argc, char **argv);
/*
 * recurse_deep_search() - get the value of the particular property in the particular node.
 *
 * 1st par. - address of the fdt tree blob
 * 2nd par. - name of node to find
 * 3rd par. - property name to get value of
 * 4th par. - the pointer to store the length of memory with value in
 * return   - the pointer to the begining of memory with value
 */
const void* recurse_deep_search(const void*, int, const char*, const char*, int*); //deep evation of the tree
uint32_t convert_fdt32( uint32_t );

#endif /*_STARTUP_FDT_H*/
