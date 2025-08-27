/*
 * Cliente MQTT simplificado para ESP32-C6
 * Autor: Edwar David Valencia Gomez
 * Universidad de Antioquia, Sistemas Embebidos
 * Curso: Sistemas IoT  
 * Período: 2025-I
 */

#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MQTT_CLIENT";

static bool mqtt_connected = false;

esp_err_t mqtt_client_init(void) {
    ESP_LOGI(TAG, "Cliente MQTT inicializado (modo simulado)");
    mqtt_connected = true;
    return ESP_OK;
}

esp_err_t mqtt_publish_sensor_data(sensor_data_t *data) {
    if (!mqtt_connected) {
        ESP_LOGW(TAG, "MQTT no conectado");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Publicando datos del sensor:");
    ESP_LOGI(TAG, "  Temperatura: %.2f°C", data->temperature);
    ESP_LOGI(TAG, "  Humedad: %.2f%%", data->humidity);
    ESP_LOGI(TAG, "  Timestamp: %lu", (unsigned long)data->timestamp);
    
    return ESP_OK;
}

esp_err_t mqtt_publish_status(const char *status) {
    if (!mqtt_connected) {
        ESP_LOGW(TAG, "MQTT no conectado");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Estado publicado: %s", status);
    return ESP_OK;
}

void mqtt_process_command(const char *topic, const char *data) {
    ESP_LOGI(TAG, "Comando procesado: %s -> %s", topic, data);
}

bool mqtt_is_connected(void) {
    return mqtt_connected;
}

void mqtt_client_deinit(void) {
    mqtt_connected = false;
    ESP_LOGI(TAG, "Cliente MQTT deinicializado");
}
