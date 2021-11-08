#ifndef _AGG_SDL2_HAL_H
#define _AGG_SDL2_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* HAL must provide display dimensions and depth. */
#define DISPLAY_WIDTH   (240)
#define DISPLAY_HEIGHT  (320)
#define DISPLAY_SCALE   (1)

void agg_hal_init(void);
void agg_hal_flush(void);
void agg_hal_close(void);
void agg_hal_test(void);

#ifdef __cplusplus
}
#endif
#endif /* _AGG_SDL2_HAL_H */
