#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Start the on-demand synthetic touch input TCP server (no-op if
 * CONFIG_DEBUG_INPUT_ENABLED is not set). The server listens on
 * CONFIG_DEBUG_INPUT_PORT and registers a synthetic LVGL pointer input
 * device. Each connection sends a stream of 5-byte commands:
 *   byte 0:   command ('T' = tap, 'D' = press/down, 'M' = move, 'U' = release/up)
 *   bytes 1-2: x coordinate (uint16, little-endian)
 *   bytes 3-4: y coordinate (uint16, little-endian)
 *
 * 'T' presses at (x, y), holds briefly, then releases — a single tap.
 * 'D' presses (and holds) at (x, y). 'M' moves the current press to
 * (x, y). 'U' releases (x, y still required but unused). */
esp_err_t debug_input_start(void);

#ifdef __cplusplus
}
#endif
