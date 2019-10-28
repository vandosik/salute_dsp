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
    uint32_t mode;
    
    mode = *MMIO32( LCDMODE );
    mode |= LCDMODE_HWCEN | LCDMODE_HWC_MODE_64x64;
    *MMIO32( LCDMODE ) = mode;
}


void vpout_disable_hw_cursor( disp_adapter_t *adapter, int dispno )
{
    vpout_context_t         *vpout        = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw   = adapter->gd_ctx;
    uint32_t mode;

    mode = *MMIO32( LCDMODE );
    mode &= ~LCDMODE_HWCEN;
    *MMIO32( LCDMODE ) = mode;
}


void vpout_set_hw_cursor_pos( disp_adapter_t *adapter, int dispno, int x, int y )
{
    vpout_context_t         *vpout        = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw   = adapter->gd_ctx;

    if ( x < 0 )
        x = -x | 0x8000;
    if ( y < 0 )
        y = -y | 0x8000;

    *MMIO32( LCDXY ) = (x & 0xFFFF) | (y << 16);
}


int vpout_set_hw_cursor( disp_adapter_t *adapter, int dispno, uint8_t *bmp0, uint8_t *bmp1, unsigned color0, unsigned color1,
                         int hotspot_x, int hotspot_y, int size_x, int size_y, int bmp_stride )
{
    vpout_context_t         *vpout         = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw    = adapter->gd_ctx;
    uint8_t                 *cursor_ptr    = NULL;
    int                     x              = 0,
                            y              = 0,
                            byte           = 0,
                            bit            = 0,
                            code           = 0;
    uint32_t                cursor_sz      = 0;
    unsigned cursor_stride;
    uint32_t mode;

    /* Fallback into software if we can't handle oversized cursor */
    if ( size_x > 64 || size_y > 64 )
        return (-1);

    cursor_sz              = 64;
    
    /* Need to enable cursor before updating it's contents */
    mode = *MMIO32( LCDMODE );
    mode |= LCDMODE_HWCEN | LCDMODE_HWC_MODE_64x64;
    *MMIO32( LCDMODE ) = mode;

    cursor_ptr = (uint8_t *)(MMIO32(HWC_MEM));
    
    for ( y = 0; y < cursor_sz; y++ )
    {
        byte = y * bmp_stride;
        bit  = 0x80;

        for ( x = 0; x < cursor_sz; x++ )
        {
            unsigned off;
            code = 0;

            if ( y < size_y && x < size_x )
            {
                if ( bmp0[byte] & bit )
                    code = 2;
                if ( bmp1[byte] & bit )
                    code = 3;
            }

            off = (x & 3) * 2;
            cursor_ptr[(y * cursor_sz + x) >> 2] &= ~(3 << off);
            cursor_ptr[(y * cursor_sz + x) >> 2] |= (code << off);

            bit >>= 1;
            if ( bit == 0 )
            {
                bit = 0x80;
                byte++;
            }
        }
    }

    *MMIO32( LCDCOLOR0 ) = color0;
    *MMIO32( LCDCOLOR1 ) = color1;
    *MMIO32( LCDXYP ) = hotspot_x | (hotspot_y << 16);

    return (0);
}
#endif  /* ENABLE_HW_CURSOR */
