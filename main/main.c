#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "pcf85063a.h"
#include "bsp/esp-bsp.h" // Donanım destek paketi
#include "bsp/display.h" // Ekran kontrolleri
#include "lvgl.h"        // Grafik kütüphanesi

static const char *TAG = "SAAT";

static pcf85063a_dev_t rtc_dev;
pcf85063a_datetime_t now_time;
static lv_obj_t *time_label;

void update_time_task(void *pvParameters)
{
    while (1)
    {
        if (pcf85063a_get_time_date(&rtc_dev, &now_time) == ESP_OK)
        {
            bsp_display_lock(0); //
            lv_label_set_text_fmt(time_label, "%02d:%02d:%02d",
                                  now_time.hour, now_time.min, now_time.sec);
            bsp_display_unlock(); //
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Sistem BSP ile başlatılıyor...");
    /* 1. Donanım ve Ekran Başlatma */
    // Bu fonksiyon I2C, AXP2101 ve SH8601 sürücüsünü otomatik kurar
    bsp_display_start();
    /* 2. Parlaklık Ayarı */
    bsp_display_brightness_set(100);

    /* 3. LVGL Arayüzü Oluşturma */
    // LVGL işlemlerini yaparken mutlaka kilitleme (lock) kullanmalısın
    bsp_display_lock(0);

    i2c_master_bus_handle_t bus_handle = bsp_i2c_get_handle(); // Mevcut bus handle'ı buraya almalısın
    // RTC'yi başlat (Adres genelde 0x51'dir)
    pcf85063a_init(&rtc_dev, bus_handle, PCF85063A_ADDRESS);

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0); // Siyah arka plan

    time_label = lv_label_create(scr);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0x00FF00), 0);
    lv_obj_center(time_label);
    bsp_display_unlock();

    // Saati bir kereliğine ayarlıyoruz
    pcf85063a_datetime_t time_to_set = {
        .year = 2026,
        .month = 1,
        .day = 31,
        .hour = 19,
        .min = 00,
        .sec = 0};
    esp_err_t set_ret = pcf85063a_set_time_date(&rtc_dev, time_to_set);
    if (set_ret == ESP_OK)
    {
        ESP_LOGI("RTC", "Saat basariyla 19:00:00 olarak ayarlandi.");
    }

    // 4. Görevi Başlat
    xTaskCreate(update_time_task, "update_time", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Arayüz yüklendi.");

    while (1)
    {
        // Not: Pil voltajı okumak için BSP'nin kendi PMU fonksiyonlarını
        // veya I2C bus handle'ını kullanabilirsin.
        ESP_LOGI(TAG, "Guncel Saat: %02d:%02d:%02d",
                 now_time.hour, now_time.min, now_time.sec);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}