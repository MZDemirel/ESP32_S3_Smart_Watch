#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "display.h"
#include "axp2101.h"

i2c_master_bus_handle_t bus_handle;
uint16_t battery_voltage_mv = 0;

void init_i2c_bus()
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = (gpio_num_t)15,
        .scl_io_num = (gpio_num_t)14,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        }};

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));
}

static const char *TAG = "SAAT";

void app_main(void)
{
    ESP_LOGI(TAG, "Sistem başlatılıyor");
    init_i2c_bus();
    ESP_LOGI(TAG, "I2C bus başlatıldı");
    ESP_ERROR_CHECK(axp2101_init(bus_handle));
    ESP_LOGI(TAG, "AXP2101 PMU başlatıldı");
    display_hardware_reset();

    ESP_ERROR_CHECK(display_init());

    ESP_LOGI("SAAT", "Donanım hazır, QSPI hattına geçilebilir.");

    ESP_LOGI("SAAT", "Ekrana renk basiliyor...");
    display_fill_color(0xF800);

    while (1)
    {
        ESP_LOGI(TAG, "Saat çalışıyor");

        esp_err_t ret = axp2101_get_battery_voltage(&battery_voltage_mv);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Pil Voltajı: %d mV", battery_voltage_mv);
        }
        else
        {
            ESP_LOGE(TAG, "Pil voltajı okunamadı: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
