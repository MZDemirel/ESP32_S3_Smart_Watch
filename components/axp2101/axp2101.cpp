#include "axp2101.h"

i2c_master_dev_handle_t dev_handle;
static const char *TAG = "AXP2101";

esp_err_t axp2101_init(i2c_master_bus_handle_t bus_handle)
{
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x34,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    uint8_t adc_en_data[] = {0x80, 0x03};
    i2c_master_transmit(dev_handle, adc_en_data, sizeof(adc_en_data), -1);

    // 3. Voltaj Seviyelerini Ayarla (Ekran için)
    // Register 0x93: ALDO2 Voltajı (0x1C = 3.3V)
    uint8_t aldo2_vol[] = {0x93, 0x1C};
    i2c_master_transmit(dev_handle, aldo2_vol, 2, -1);

    // Register 0x96: BLDO1 Voltajı (0x0D = 1.8V)
    uint8_t bldo1_vol[] = {0x96, 0x0D};
    i2c_master_transmit(dev_handle, bldo1_vol, 2, -1);

    // Register 0x99: DLDO1 Voltajı (0x1C = 3.3V)
    uint8_t dldo1_vol[] = {0x99, 0x1C};
    i2c_master_transmit(dev_handle, dldo1_vol, 2, -1);

    // 4. Kanalları Aktif Et (Power Enable)
    // Register 0x90: LDO ve DC-DC kanallarını açma/kapama
    // 0xBF değeri ALDO1, ALDO2, BLDO1, DLDO1 ve DC-DC'leri açar
    uint8_t pwr_en[] = {0x90, 0xBF};
    esp_err_t ret = i2c_master_transmit(dev_handle, pwr_en, 2, -1);

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "ADC aktif edildi.");
    }

    return ret;
}

esp_err_t axp2101_get_battery_voltage(uint16_t *voltage_mv)
{
    if (voltage_mv == NULL)
        return ESP_ERR_INVALID_ARG;

    uint8_t reg_addr = 0x34;
    uint8_t read_buf[2];

    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg_addr, 1, read_buf, 2, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read battery voltage : %s", esp_err_to_name(ret));
        return ret;
    }

    uint16_t raw_voltage = (read_buf[0] << 6) | (read_buf[1] & 0x3F);
    *voltage_mv = raw_voltage;
    return ESP_OK;
}