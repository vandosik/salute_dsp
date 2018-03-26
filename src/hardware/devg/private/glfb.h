#ifndef _GLFB_H_
#define _GLFB_H_

struct glfb_ctx;
typedef struct glfb_ctx *glfb_ctx_t;

glfb_ctx_t glfb_init( disp_adapter_t *adp );
void glfb_fini( glfb_ctx_t ctx );

int glfb_get_contextfuncs(disp_adapter_t *adapter,
    disp_draw_contextfuncs_t *funcs, int tabsize);

#endif /* _GLFB_H_ */
