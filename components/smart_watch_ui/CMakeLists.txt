idf_component_register(SRCS "smart_watch_ui.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES driver 
                             esp_lcd
                             axp2101 
                             waveshare__esp_lcd_sh8601
                             lvgl 
                             esp_lvgl_port
                             waveshare__pcf85063a
                             smart_watch_ui
                             waveshare__esp32_s3_touch_amoled_2_06)