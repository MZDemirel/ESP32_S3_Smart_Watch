#include "display.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_sh8601.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LCD_RESET_PIN (gpio_num_t)8 // Şemadaki LCD_RESET pini
#define LCD_QSPI_SCLK 11
#define LCD_QSPI_CS 12
#define LCD_QSPI_D0 4
#define LCD_QSPI_D1 5
#define LCD_QSPI_D2 6
#define LCD_QSPI_D3 7

esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;
static const char *TAG = "DISPLAY";

esp_err_t display_hardware_reset(void)
{
    // 1. Reset pinini çıkış olarak yapılandır
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LCD_RESET_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // 2. Hardware Reset Sequence (CO5300 Zamanlaması)
    ESP_LOGI(TAG, "Ekran resetleniyor...");
    gpio_set_level(LCD_RESET_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(LCD_RESET_PIN, 0); // Reset'i çek
    vTaskDelay(pdMS_TO_TICKS(20));    // En az 20ms bekle
    gpio_set_level(LCD_RESET_PIN, 1); // Reset'i bırak
    vTaskDelay(pdMS_TO_TICKS(120));   // Çipin kendine gelmesi için 120ms bekle

    ESP_LOGI(TAG, "Ekran donanımsal olarak uyanmaya hazır.");
    return ESP_OK;
}

esp_err_t display_init(void)
{
    ESP_LOGI(TAG, "QSPI Bus yapilandiriliyor...");

    // 1. SPI Veriyolu (Bus) Yapılandırması
    spi_bus_config_t buscfg = {
        .data0_io_num = LCD_QSPI_D0,
        .data1_io_num = LCD_QSPI_D1,
        .sclk_io_num = LCD_QSPI_SCLK,
        .data2_io_num = LCD_QSPI_D2,
        .data3_io_num = LCD_QSPI_D3,
        .max_transfer_sz = 454 * 454 * 2,                           // Ekran çözünürlüğü kadar buffer
        .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_QUAD, // QUAD Modu Aktif
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // 2. LCD Panel IO (Giriş/Çıkış) Ayarları
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_QSPI_CS,
        .dc_gpio_num = -1, // QSPI modunda DC pini genellikle yoktur, komutlar data hattından gider
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000, // 40MHz hız
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {
            .quad_mode = true, // Quad SPI olduğunu belirtiyoruz
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Panel sürücüsü oluşturuluyor...");

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1, // Reset işlemini manuel yaptığımız için -1
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16, // RGB565 formatı
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(io_handle, &panel_config, &panel_handle));

    // Standart başlatma
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // AMOLED spesifik uykudan çıkış süresi
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "QSPI hatti basariyla kuruldu.");
    return ESP_OK;
}

esp_err_t display_fill_color(uint16_t color)
{
    if (panel_handle == NULL)
        return ESP_ERR_INVALID_STATE;

    int width = 454;
    int height = 454;

    // DMA uyumlu bellek ayıralım
    uint16_t *buffer = (uint16_t *)heap_caps_malloc(width * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    uint16_t swapped_color = (color << 8) | (color >> 8);
    for (int i = 0; i < width; i++)
        buffer[i] = swapped_color;

    for (int y = 0; y < height; y++)
    {
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y, width, y + 1, buffer);
    }

    free(buffer);
    return ESP_OK;
}