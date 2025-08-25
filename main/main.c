/*
 * ESP32-C6 DHT11 Sensor con Mosquitto MQTT en AWS EC2
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 * 
 * Descripción: Sistema IoT para lectura de sensor DHT11 y envío de datos
 * via MQTT a broker Mosquitto en Amazon EC2 con interfaz web local.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

// Incluir modulos
#include "config.h"
#include "wifi_config.h"
#include "sensor_manager.h"
#include "mqtt_client.h"
#include "http_server.h"

static const char *TAG = "MAIN";

// Callback para enviar datos cuando se obtienen nuevas lecturas del sensor
void sensor_data_callback(sensor_data_t *data) {
    ESP_LOGI(TAG, "Nuevos datos del sensor recibidos");
    
    // Enviar via MQTT
    if (mqtt_is_connected()) {
        mqtt_publish_sensor_data(data);
    } else {
        ESP_LOGW(TAG, "MQTT no conectado, datos no enviados");
    }
    
    // También enviar via HTTP a EC2
    esp_err_t result = http_send_to_aws_ecs(data);
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Datos DHT11 enviados a EC2 exitosamente");
    } else {
        ESP_LOGW(TAG, "Error enviando datos DHT11 a EC2");
    }
}

void app_main(void) {
    ESP_LOGI(TAG, " Iniciando ESP32-C6 con sensor DHT11 y conexión a Mosquitto EC2");
    ESP_LOGI(TAG, " Autor: Edward Fabian Goyeneche Velandia");
    ESP_LOGI(TAG, " Universidad Nacional de Colombia - Sede Manizales");
    ESP_LOGI(TAG, " Grupo Percepción de Control Inteligente");
    ESP_LOGI(TAG, " Asignatura: Desarrollo de Sistemas IoT - 2025-II");
    ESP_LOGI(TAG, "========================================================");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Mostrar información del sistema
    ESP_LOGI(TAG, "   Información del sistema:");
    ESP_LOGI(TAG, "   Chip: ESP32-C6 DevKitC-1");
    ESP_LOGI(TAG, "   Sensor: DHT11 en GPIO%d", DHT11_GPIO_PIN);
    ESP_LOGI(TAG, "   Memoria libre: %lu bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "   Versión IDF: %s", esp_get_idf_version());
    
    // Paso 1: Inicializar WiFi
    ESP_LOGI(TAG, " Inicializando WiFi...");
    ESP_ERROR_CHECK(wifi_init_sta());
    
    esp_netif_ip_info_t ip_info;
    if (wifi_get_ip_info(&ip_info) == ESP_OK) {
        ESP_LOGI(TAG, "✅ WiFi conectado - IP: " IPSTR, IP2STR(&ip_info.ip));
    }
    
    // Paso 2: Inicializar sensor DHT11
    ESP_LOGI(TAG, " Inicializando sensor DHT11...");
    ESP_ERROR_CHECK(sensor_manager_init());
    
    // Registrar callback para envío automático de datos
    sensor_register_callback(sensor_data_callback);
    ESP_LOGI(TAG, " Sensor DHT11 inicializado y callback registrado");
    
    // Paso 3: Inicializar cliente MQTT con autenticación
    ESP_LOGI(TAG, " Inicializando cliente MQTT con Mosquitto...");
    ESP_ERROR_CHECK(mqtt_client_init());
    ESP_LOGI(TAG, " Cliente MQTT inicializado con autenticación");
    
    // Paso 4: Inicializar servidor HTTP
    ESP_LOGI(TAG, " Inicializando servidor HTTP...");
    ESP_ERROR_CHECK(http_server_init());
    ESP_LOGI(TAG, " Servidor HTTP iniciado en puerto %d", HTTP_SERVER_PORT);
    
    // Mostrar información de acceso
    ESP_LOGI(TAG, " ¡Sistema DHT11 iniciado exitosamente!");
    ESP_LOGI(TAG, " Acceso local: http://" IPSTR ":%d", IP2STR(&ip_info.ip), HTTP_SERVER_PORT);
    ESP_LOGI(TAG, " API endpoints:");
    ESP_LOGI(TAG, "    Datos del DHT11: /api/sensor");
    ESP_LOGI(TAG, "     Estado del sistema: /api/status");
    ESP_LOGI(TAG, "     Envío manual a EC2: /api/aws-send");
    ESP_LOGI(TAG, " Mosquitto Broker: %s:%d", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    ESP_LOGI(TAG, " Con autenticación: usuario=%s", MQTT_USERNAME);
    ESP_LOGI(TAG, " Topics MQTT:");
    ESP_LOGI(TAG, "    Datos DHT11: %s", MQTT_TOPIC_SENSOR_DATA);
    ESP_LOGI(TAG, "    Estado: %s", MQTT_TOPIC_STATUS);
    ESP_LOGI(TAG, "    Comandos: esp32/dht11/commands");
    ESP_LOGI(TAG, " WebSocket disponible en puerto: %d", WEBSOCKET_PORT);
    
    // Esperar un poco para que todo se estabilice
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Enviar estado inicial
    mqtt_publish_status("system_started");
    
    // Loop principal
    while (1) {
        // Verificar estado de conexiones
        if (!wifi_is_connected()) {
            ESP_LOGW(TAG, " WiFi desconectado, intentando reconectar...");
            wifi_init_sta();
        }
        
        if (!mqtt_is_connected()) {
            ESP_LOGW(TAG, " MQTT desconectado");
        }
        
        // Mostrar estadísticas del sistema cada 30 segundos
        ESP_LOGI(TAG, " Memoria libre: %lu bytes", esp_get_free_heap_size());
        
        // Esperar 30 segundos antes de la siguiente verificación
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}
