# Configuración de componentes del proyecto
# Autor: Edward Fabian Goyeneche Velandia
# Universidad Nacional de Colombia - Sede Manizales
# Grupo Percepción de Control Inteligente
# Asignatura: Desarrollo de Sistemas IoT
# Período: 2025-II

idf_component_register(SRCS "main.c" "sensor_manager.c" "mqtt_client.c" "http_server.c" "wifi_config.c"
                       INCLUDE_DIRS "."
                       REQUIRES "esp_wifi" "nvs_flash" "esp_http_server" "spiffs" "esp_http_client" "esp_timer" "driver" "esp_system" "esp_event" "esp_rom" "mqtt" "json" "freertos"
                       PRIV_REQUIRES "json")

# Configuración para archivos web embebidos
spiffs_create_partition_image(storage ../web FLASH_IN_PROJECT)
