#pragma once

#include "hal_display.h"
#include "hal_touch.h"
#include "hal_storage.h"

typedef struct {
    hal_display_t  *display;
    hal_touch_t    *touch;
} hal_handles_t;

extern hal_handles_t g_hal;
