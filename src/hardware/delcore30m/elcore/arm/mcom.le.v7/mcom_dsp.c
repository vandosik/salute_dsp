#include <stdio.h>
#include <stdlib.h>
#include <elcore-manager.h>
#include "mcom_dsp.h"

void *elcore_func_init(void *hdl, char *options)
{
    delcore30m_t			*dev;
    
    dev = calloc(1, sizeof(delcore30m_t));
    if ( dev == NULL )
    {
        printf("%s: error\n", __func__);
        return NULL;
    }
    
    dev->drvhdl.hdl = hdl;
    
    printf("%s: success\n", __func__);
    
    return dev;
    
}
void elcore_func_fini(void *hdl)
{
    printf("%s: success\n", __func__);
    
    free(hdl);
}
