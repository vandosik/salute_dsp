/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


int devg_get_corefuncs( disp_adapter_t *adapter, unsigned pixel_format, disp_draw_corefuncs_t *funcs, int tabsize )
{
    /* Get the default software drawing routines first. */
    if ( ffb_get_corefuncs( adapter, pixel_format, funcs, tabsize ) == -1 )
        return (-1);

    return (0);
}
