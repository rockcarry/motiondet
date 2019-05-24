#ifndef __MOTIONDET_H__
#define __MOTIONDET_H__

#include <stdint.h>

void *motiondet_init(uint16_t *grids, int w, int h, int sensitivity);
void  motiondet_exit(void *ctxt);
int   motiondet_data(void *ctxt, uint8_t *data, uint32_t diff);
void  motiondet_sensitivity(void *ctxt, int sensitivity);

#endif
