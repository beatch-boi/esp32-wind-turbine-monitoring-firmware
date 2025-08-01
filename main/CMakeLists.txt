idf_component_register(SRCS "main.c" "wifi/wifi.c" "http-server/http-server.c" "filesystem/filesystem.c" "i2c/i2c.c" "adc/adc.c" "blade-speed-sensor/blade-speed-sensor.c" "motor-speed-sensor/motor-speed-sensor.c"
                    INCLUDE_DIRS ".")

spiffs_create_partition_image("fs" "../fs" FLASH_IN_PROJECT)