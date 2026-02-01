#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "axp2101.h"
#include "smart_watch_ui.h"
#include "pcf85063a.h"
#include "bsp/esp-bsp.h" // Donanım destek paketi
#include "bsp/display.h" // Ekran kontrolleri
#include "lvgl.h"        // Grafik kütüphanesi

static const char *TAG = "SAAT";

pcf85063a_dev_t rtc_dev;
pcf85063a_datetime_t now_time;
lv_obj_t *time_label;
lv_obj_t *bat_label;

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Sistem BSP ile başlatılıyor...");
    /* 1. Donanım ve Ekran Başlatma */
    // Bu fonksiyon I2C, AXP2101 ve SH8601 sürücüsünü otomatik kurar
    bsp_display_start();
    /* 2. Parlaklık Ayarı */
    bsp_display_brightness_set(100);

    i2c_master_bus_handle_t bus_handle = bsp_i2c_get_handle(); // Mevcut bus handle'ı buraya almalısın
    // RTC'yi başlat (Adres genelde 0x51'dir)
    pcf85063a_init(&rtc_dev, bus_handle, PCF85063A_ADDRESS);
    axp2101_init(bus_handle);

    /* 3. LVGL Arayüzü Oluşturma */
    // LVGL işlemlerini yaparken mutlaka kilitleme (lock) kullanmalısın
    bsp_display_lock(0);
    // Aktif ekranı al ve arka planı siyah yap
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    // Saat etiketi: Fontu büyüt ve rengini değiştir
    time_label = lv_label_create(scr);
    // Not: Montserrat 48 fontunun sdkconfig'de açık olması gerekir
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0x00FF00), 0); // Yeşil
    lv_obj_center(time_label);

    // Pil etiketi: Sağ üst köşeye beyaz renkle yerleştir
    bat_label = lv_label_create(scr);
    lv_obj_set_style_text_font(bat_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bat_label, lv_color_hex(0xFFFFFF), 0); // Beyaz
    lv_obj_align(bat_label, LV_ALIGN_TOP_RIGHT, -40, 40);

    bsp_display_unlock();

    pcf85063a_datetime_t check_time;
    esp_err_t ret = pcf85063a_get_time_date(&rtc_dev, &check_time);

    // Eğer yıl 2024'ten küçükse veya okuma hatası varsa saati kur
    if (ret != ESP_OK || check_time.year < 2024)
    {
        ESP_LOGW(TAG, "Saat gecersiz! Yeniden ayarlaniyor...");
        pcf85063a_datetime_t initial_time = {
            .year = 2026, .month = 1, .day = 31, .hour = 12, .min = 0, .sec = 0};
        pcf85063a_set_time_date(&rtc_dev, initial_time);
    }
    else
    {
        ESP_LOGI(TAG, "Saat dogru sekilde devam ediyor.");
    }

    // 4. Görevi Başlat
    xTaskCreate(update_ui_task, "update_ui", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Arayüz yüklendi.");

    while (1)
    {
        uint8_t bat_per = 0;
        uint16_t bat_volt = 0;

        if (axp2101_get_battery_percentage(&bat_per) == ESP_OK &&
            axp2101_get_battery_voltage(&bat_volt) == ESP_OK)
        {
            ESP_LOGI(TAG, "Pil: %%%d - Voltaj: %dmV", bat_per, bat_volt);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}