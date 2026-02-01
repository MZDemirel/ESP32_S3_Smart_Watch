idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES axp2101 
                             smart_watch_ui 
                             qmi8658 
                             waveshare__esp_lcd_sh8601 
                             waveshare__esp32_s3_touch_amoled_2_06 
                             lvgl 
                             esp_lvgl_port
                             waveshare__pcf85063a)