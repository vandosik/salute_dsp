/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


/* Populate the function table of core drawing routines. tabsize is the size of the function table in bytes.
 * pixel_format specifies the surface type that the draw routines must be able to render to */
int devg_get_contextfuncs( disp_adapter_t *adapter, disp_draw_contextfuncs_t *funcs, int tabsize )
{
    /* Get the default software functions first */
    if ( ffb_get_contextfuncs( adapter, funcs, tabsize ) == -1 )
        return (-1);

    return (0);
}
