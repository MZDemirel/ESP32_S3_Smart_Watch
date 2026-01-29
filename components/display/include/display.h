#ifndef DISPLAY_H
#define DISPLAY_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Ekranın reset pinini yöneten ve donanımı başlatan fonksiyon
    esp_err_t display_hardware_reset(void);

    esp_err_t display_init(void);

    esp_err_t display_fill_color(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_H