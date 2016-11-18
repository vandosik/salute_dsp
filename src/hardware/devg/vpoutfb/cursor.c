/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


#ifdef ENABLE_HW_CURSOR
void vpout_enable_hw_cursor( disp_adapter_t *adapter, int dispno )
{
    vpout_context_t         *vpout        = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw   = adapter->gd_ctx;
    int                     pipe          = dispno;

    if ( VPOUT_DISPMODE_BAD_PIPE( pipe ) )
        return;

    //~ 
}


void vpout_disable_hw_cursor( disp_adapter_t *adapter, int dispno )
{
    vpout_context_t         *vpout        = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw   = adapter->gd_ctx;
    int                     pipe          = dispno;

    if ( VPOUT_DISPMODE_BAD_PIPE( pipe ) )
        return;

    pipe = dispno;

    //~ 
}


void vpout_set_hw_cursor_pos( disp_adapter_t *adapter, int dispno, int x, int y )
{
    vpout_context_t         *vpout        = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw   = adapter->gd_ctx;
    int                     pipe          = dispno;
    int                     ox            = 0;
    int                     oy            = 0;

    ox = x - vpout->cursor_hs_x;
    oy = y - vpout->cursor_hs_y;

    if ( VPOUT_DISPMODE_BAD_PIPE( pipe ) )
        return;

    if ( ox < 0 )
        ox = -ox | 0x8000;
    if ( oy < 0 )
        oy = -oy | 0x8000;

    //~ 
}


int vpout_set_hw_cursor( disp_adapter_t *adapter, int dispno, uint8_t *bmp0, uint8_t *bmp1, unsigned color0, unsigned color1,
                         int hotspot_x, int hotspot_y, int size_x, int size_y, int bmp_stride )
{
    vpout_context_t         *vpout         = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw    = adapter->gd_ctx;
    int                     pipe           = dispno;
    uint8_t                 *cursor_ptr    = NULL;
    int                     x              = 0,
                            y              = 0,
                            byte           = 0,
                            bit            = 0,
                            code           = 0;
    uint32_t                cursor_sz      = 0;

    if ( VPOUT_DISPMODE_BAD_PIPE( pipe ) )
        return (-1);

    /* Fallback into software if we can't handle oversized cursor */
    if ( size_x > 64 || size_y > 64 )
        return (-1);

    cursor_sz              = 64;

    vpout->cursor_hs_x = hotspot_x;
    vpout->cursor_hs_y = hotspot_y;
    cursor_ptr = (uint8_t *)((unsigned int)vpout_draw->system_buf_vptr /*+ vpout->cursor_base*/ + 0);

    
    for ( y = 0; y < cursor_sz; y++ )
    {
        byte = y * bmp_stride;
        bit  = 0x80;

        for ( x = 0; x < cursor_sz; x++ )
        {
            code = 2;

            if ( y < size_y && x < size_x )
            {
                if ( bmp0[byte] & bit )
                    code = 0;
                if ( bmp1[byte] & bit )
                    code = 1;
            }

            switch ( code )
            {
                case 0:
                     /* color 0 */
                     cursor_ptr[(y * 128 + x) >> 3]             &= ~bit;
                     cursor_ptr[(y * 128 + cursor_sz + x) >> 3] &= ~bit;
                     break;
                case 1:
                     /* color 1 */
                     cursor_ptr[(y * 128 + x) >> 3]             &= ~bit;
                     cursor_ptr[(y * 128 + cursor_sz + x) >> 3] |= bit;
                     break;
                case 2:
                     /* transparent */
                     cursor_ptr[(y * 128 + x) >> 3]             |= bit;
                     cursor_ptr[(y * 128 + cursor_sz + x) >> 3] &= ~bit;
                     break;
            }

            bit >>= 1;
            if ( bit == 0 )
            {
                bit = 0x80;
                byte++;
            }
        }
    }

    //~ 

    return (0);
}
#endif  /* ENABLE_HW_CURSOR */
