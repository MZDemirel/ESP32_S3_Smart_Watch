#ifndef AXP2101_H
#define AXP2101_H

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Initialize AXP2101 PMU
     * @param bus_handle I2C master bus handle
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t axp2101_init(i2c_master_bus_handle_t bus_handle);

    /**
     * @brief Get battery voltage from AXP2101
     * @param voltage_mv Pointer to store battery voltage in millivolts
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t axp2101_get_battery_voltage(uint16_t *voltage_mv);

#ifdef __cplusplus
}
#endif
#endif // AXP2101_H