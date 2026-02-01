#include "axp2101.h"
#include "esp_log.h"

static i2c_master_dev_handle_t dev_handle;
static const char *TAG = "AXP2101";

// 1. DÜZENLEME: Sadece cihazı kayıt et ve ADC'yi kontrol et
esp_err_t axp2101_init(i2c_master_bus_handle_t bus_handle)
{
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x34, // AXP2101 Standart Adresi
        .scl_speed_hz = 100000,
    };

    // Cihazı mevcut I2C hattına ekle
    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
    if (ret != ESP_OK)
        return ret;

    // Sadece ADC'yi aktif et (Pil okumak için gereklidir)
    // Register 0x80: ADC enable
    uint8_t adc_en_data[] = {0x80, 0x03};
    return i2c_master_transmit(dev_handle, adc_en_data, sizeof(adc_en_data), -1);
}

// 2. DÜZENLEME: Pil Voltajı Okuma (Register 0x34-0x35)
esp_err_t axp2101_get_battery_voltage(uint16_t *voltage_mv)
{
    uint8_t reg_addr = 0x34;
    uint8_t read_buf[2];

    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg_addr, 1, read_buf, 2, 1000);
    if (ret == ESP_OK)
    {
        // AXP2101 için 14-bit voltaj hesaplaması: (MSB << 6) | (LSB & 0x3F)
        // Çarpan genelde 1mV'dur.
        *voltage_mv = (read_buf[0] << 6) | (read_buf[1] & 0x3F);
    }
    return ret;
}

// 3. YENİ: Pil Yüzdesi Okuma (Register 0xA1)
esp_err_t axp2101_get_battery_percentage(uint8_t *percent)
{
    uint8_t reg_addr = 0xA4; // Fuel Gauge register
    uint8_t val;
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg_addr, 1, &val, 1, 1000);
    if (ret == ESP_OK)
    {
        *percent = (val); // 0-100 arası değer döner
    }
    return ret;
}

uint8_t axp2101_get_charge_status(void)
{
    uint8_t reg_addr = 0x01; // XPOWERS_AXP2101_STATUS2
    uint8_t val;
    if (i2c_master_transmit_receive(dev_handle, &reg_addr, 1, &val, 1, 1000) == ESP_OK)
    {
        // Bit 5 ve 6'yı kontrol ediyoruz (xpowerlib'deki >> 5 mantığı)
        return (val >> 5) & 0x03;
    }
    return 3;
}