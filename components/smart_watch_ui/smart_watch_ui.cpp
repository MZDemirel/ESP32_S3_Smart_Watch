#include "axp2101.h"
#include "pcf85063a.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "lvgl.h"
#include "smart_watch_ui.h"

// main.cpp dosyasında tanımlanan global objelere erişim
extern pcf85063a_dev_t rtc_dev;
extern lv_obj_t *time_label;
extern lv_obj_t *bat_label;

void update_ui_task(void *pvParameters)
{
    pcf85063a_datetime_t now;
    uint8_t bat_per = 0;
    uint8_t charge_status = axp2101_get_charge_status();

    while (1)
    {
        // 1. Verileri donanımdan oku
        esp_err_t rtc_ret = pcf85063a_get_time_date(&rtc_dev, &now);
        esp_err_t bat_ret = axp2101_get_battery_percentage(&bat_per);

        // 2. UI Kilidini aç ve etiketleri güncelle
        bsp_display_lock(0);

        // Zamanı güncelle
        if (rtc_ret == ESP_OK && time_label != NULL)
        {
            lv_label_set_text_fmt(time_label, "%02d:%02d:%02d",
                                  now.hour, now.min, now.sec);
        }

        // Pil yüzdesini güncelle
        if (bat_ret == ESP_OK && bat_label != NULL)
        {
            if (charge_status == 1)
            { // Şarj oluyor
                lv_label_set_text_fmt(bat_label, LV_SYMBOL_CHARGE " %% %d", (bat_per & 0x7F));
                lv_obj_set_style_text_color(bat_label, lv_color_hex(0xFFFF00), 0); // Şarj olurken sarı yap
            }
            else if (charge_status == 2)
            { // Tam dolu
                lv_label_set_text_fmt(bat_label, LV_SYMBOL_OK " %% %d", (bat_per & 0x7F));
                lv_obj_set_style_text_color(bat_label, lv_color_hex(0x00FF00), 0); // Tam dolu yeşil yap
            }
            else
            {
                lv_label_set_text_fmt(bat_label, "%% %d", (bat_per & 0x7F));
                // Deşarj olurken normal renk (yeşil/beyaz)
                lv_obj_set_style_text_color(bat_label, lv_color_hex(0xFFFFFF), 0);
            }
        }

        bsp_display_unlock();

        // 3. Bir sonraki güncelleme için bekle
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}