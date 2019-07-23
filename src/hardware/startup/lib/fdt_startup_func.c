#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "startup.h"

#include "libfdt_env.h"
#include "fdt.h"
#include "libfdt.h"

#include "fdt_startup_func.h"

uint8_t				fdt_flags = 0;
paddr32_t			fdt_addr = NULL_PADDR32;
unsigned			fdt_size = 0;

int process_fdt(int argc, char **argv)
{
	int					opt;
	char* 				fdt_options;
	void				*header = 0;

    
    while ((opt = getopt(argc, argv, COMMON_OPTIONS_STRING "Wm:a:")) != -1) {
		switch (opt) {
			case 'a':   
				fdt_options = strtok( optarg, ":" );
				if ( fdt_options != NULL ) {
					fdt_addr = strtoul(fdt_options, NULL, 0);
					do {
						fdt_options = strtok( NULL, ":" );
						if ( fdt_options != NULL ) {
							if ( !strncmp(fdt_options, "cpu", 3) ) {
							fdt_flags |= USE_FDT_CPU_PLL;
							} else if ( !strncmp(fdt_options, "mem", 3) ) {
							fdt_flags |= USE_FDT_MEM_CONFIG;
							} else if ( !strncmp(fdt_options, "sdmmc", 5) ) {
							fdt_flags |= USE_FDT_SDMMC_CONFIG;
							}
						}
					} while ( fdt_options != NULL );
					if ( !fdt_flags ) {
					fdt_flags = USE_FDT_CPU_PLL | USE_FDT_MEM_CONFIG | USE_FDT_SDMMC_CONFIG;
					}
				}
				
				break;
		}
	}
	optind = 1;

	if ( fdt_addr !=  NULL_PADDR32 ) {
		
		
		header = (void*)startup_io_map(sizeof(struct fdt_header), fdt_addr);
		
		if(fdt_check_header(header) == 0) {
			fdt_size = fdt_totalsize(header);
		}
		
		startup_io_unmap((uintptr_t)header);
		
		if(fdt_size == 0) {
			
			return EFAULT;
		}
		//Avoid using memory with fdt tree
		avoid_ram(fdt_addr, fdt_size);
		
		return EOK;
	}

	return EINVAL;
}

const void* recurse_deep_search(const void* fdt, int offset, const char* nd_name_to_fnd, const char* pr_name_to_find, int* lenp)
{
	const char      *node_name;
	int             subnode;
	int             first_flag = 1; //flag of first or nonfirst subnode
	const void*     data;    
	

	do
	{
		subnode =  first_flag ? fdt_first_subnode(fdt, offset) : fdt_next_subnode(fdt, subnode);
		
		first_flag = 0;

		if ( subnode >= 0 )
		{
			node_name = fdt_get_name(fdt, subnode, NULL);

			if ( strncmp(node_name, nd_name_to_fnd, strlen(nd_name_to_fnd)) == 0)
			{
				return fdt_getprop(fdt, subnode, pr_name_to_find , lenp);
			}
			
			data = recurse_deep_search(fdt, subnode, nd_name_to_fnd, pr_name_to_find, lenp); //parse this node's children
			if ( data )
			{
				return data;
			}
		}        
	}
	while ( subnode >= 0 );
		
	return NULL;
}


uint32_t convert_fdt32( uint32_t data)
{
	return fdt32_to_cpu( data );
}



