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

    /**
     * @brief Get battery percentage from AXP2101
     * @param percent Pointer to store battery percentage (0-100)
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t axp2101_get_battery_percentage(uint8_t *percent);

    /**
     * @brief Get charge status from AXP2101
     * @return Charge status value (0: not charging, 1: charging, 2: full, 3: error)
     */
    uint8_t axp2101_get_charge_status(void);

#ifdef __cplusplus
}
#endif
#endif // AXP2101_H