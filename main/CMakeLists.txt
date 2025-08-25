# Configuración de componentes del proyecto
# Autor: Edward Fabian Goyeneche Velandia
# Universidad Nacional de Colombia - Sede Manizales
# Grupo Percepción de Control Inteligente
# Asignatura: Desarrollo de Sistemas IoT
# Período: 2025-II

idf_component_register(SRCS "main.c" "wifi_config.c" "mqtt_client.c" "http_server.c" "sensor_manager.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_http_server mqtt nvs_flash json driver spiffs)

# Configuración para archivos web embebidos
spiffs_create_partition_image(storage ../web FLASH_IN_PROJECT)
