#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h" // Uyku fonksiyonları için
#include "axp2101.h"
#include "smart_watch_ui.h"
#include "pcf85063a.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "lvgl.h"

static const char *TAG = "SAAT";

pcf85063a_dev_t rtc_dev;
pcf85063a_datetime_t now_time;
lv_obj_t *time_label;
lv_obj_t *bat_label;

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Sistem baslatiliyor...");

    // 1. Donanim ve Ekran Baslatma
    bsp_display_start();
    bsp_display_brightness_set(100);

    i2c_master_bus_handle_t bus_handle = bsp_i2c_get_handle();
    pcf85063a_init(&rtc_dev, bus_handle, PCF85063A_ADDRESS);
    axp2101_init(bus_handle);

    // 2. LVGL Arayüzü Olusturma
    bsp_display_lock(0);
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    time_label = lv_label_create(scr);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0x00FF00), 0);
    lv_obj_center(time_label);

    bat_label = lv_label_create(scr);
    lv_obj_set_style_text_font(bat_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bat_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(bat_label, LV_ALIGN_TOP_RIGHT, -40, 40);
    bsp_display_unlock();

    // 3. RTC Kontrolü ve Ayarı
    pcf85063a_datetime_t check_time;
    // Hata 4 Çözümü: Tüm struct üyelerini sıfırlayarak başlatıyoruz
    if (pcf85063a_get_time_date(&rtc_dev, &check_time) != ESP_OK || check_time.year < 2024)
    {
        pcf85063a_datetime_t initial_time = {
            .year = 2026, .month = 1, .day = 31,
            .dotw = 0, // Eksik olan dotw eklendi
            .hour = 12,
            .min = 0,
            .sec = 0};
        pcf85063a_set_time_date(&rtc_dev, initial_time);
    }

    // 4. Arayüz Güncelleme Görevini Baslat
    xTaskCreate(update_ui_task, "update_ui", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Ekran 5 saniye acik kalacak...");

    // 5. BEKLEME SÜRESİ (5 Saniye)
    vTaskDelay(pdMS_TO_TICKS(5000));

    // 6. DERİN UYKU HAZIRLIĞI
    ESP_LOGI(TAG, "Derin uykuya geciliyor...");

    // Hata 1-2-3 Çözümü: ESP32-S3 için EXT1 uyandırma modu
    // GPIO 10 (PWR butonu) LOW seviyesine düştüğünde uyanır
    esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_10, ESP_EXT1_WAKEUP_ANY_HIGH);

    // Pil tasarrufu için ekran parlaklığını kapat
    bsp_display_brightness_set(0);

    // Derin uykuyu baslat
    esp_deep_sleep_start();
}