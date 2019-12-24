#include <stdio.h>
#include <stdlib.h>
#include "./../elcore-master.h"

void *elcore_func_init(void *hdl, char *options)
{
    ELCORE_DEV			*dev;
    
    dev = calloc(1, sizeof(ELCORE_DEV));
    if ( dev == NULL )
    {
        printf("%s: error\n", __func__);
        return NULL;
    }
    
    dev->hdl = hdl;
    
    printf("%s: success\n", __func__);
    
    return dev;
    
}
void elcore_func_fini(void *hdl)
{
    printf("%s: success\n", __func__);
    
    free(hdl);
}
