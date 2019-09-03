#include <stdlib.h>
#include <string.h>
#include "motiondet.h"

typedef struct {
    uint16_t grids [10]; // 16x10 grids bitmap, to indicate which grids need be detected
    uint16_t result[10]; // 16x10 grids bitmap result
    int      imgw;
    int      imgh;
    uint8_t *buffer;
    int      sensitivity;
} CONTEXT;

static int grid_diff(uint8_t *dstbuf, int dststride, uint8_t *srcbuf, int srcstride, int w, int h)
{
    uint32_t diff = 0, i, j;
    for (j=0; j<h; j++) {
        for (i=0; i<w; i++) {
            diff += abs(*dstbuf - *srcbuf);
            *dstbuf++ = *srcbuf++;
        }
        dstbuf += dststride - w;
        srcbuf += srcstride - w;
    }
    return diff / (w * h);
}

void *motiondet_init(uint16_t *grids, int w, int h, int sensitivity)
{
    int i, j, n, gw, gh;
    CONTEXT *context = malloc(sizeof(CONTEXT));
    if (!context) return NULL;

    if (grids) memcpy(context->grids, grids, sizeof(context->grids));
    else       memset(context->grids, 0xff , sizeof(context->grids));
    context->imgw        = w;
    context->imgh        = h;
    context->sensitivity = sensitivity;

    for (i=0,n=0; i<10; i++) {
        for (j=0; j<16; j++) {
            if (context->grids[i] & (1 << j)) n++;
        }
    }

    gw = (context->imgw + 15) / 16;
    gh = (context->imgh + 9 ) / 10;
    context->buffer = calloc(1, gw * gh * n);
    return context;
}

void motiondet_exit(void *ctxt)
{
    CONTEXT *context = (CONTEXT*)ctxt;
    if (context) {
        free(context->buffer);
        free(context);
    }
}

int motiondet_data(void *ctxt, uint8_t *data, uint32_t td)
{
    uint8_t *dstgrid, *srcgrid;
    int      det, diff, i, j, gw, gh;

    CONTEXT *context = (CONTEXT*)ctxt;
    if (!context || !context->buffer) return 0;

    dstgrid = context->buffer;
    gw      =(context->imgw + 15) / 16;
    gh      =(context->imgh + 9 ) / 10;
    det     = 0;
    for (i=0; i<10; i++) {
        for (j=0; j<16; j++) {
            if (context->grids[i] & (1 << j)) {
                srcgrid = data + j * gw + i * gh * context->imgw;
                diff    = grid_diff(dstgrid, gw, srcgrid, context->imgw, gw, gh);
                if (td < 500 && diff > context->sensitivity) {
                    context->result[i] |=  (1 << j);
                    det++;
                } else {
                    context->result[i] &= ~(1 << j);
                }
                dstgrid+= gw * gh;
            } else {
                context->result[i] &= ~(1 << j);
            }
        }
    }
    return det;
}

void motiondet_sensitivity(void *ctxt, int sensitivity)
{
    if (ctxt) ((CONTEXT*)ctxt)->sensitivity = sensitivity;
}

