/*
 * Cliente MQTT para Comunicación con Mosquitto en AWS EC2
 * Implementa la funcionalidad completa de MQTT con autenticación
 * 
 * Autor: Edwar David Valencia Gomez
 * Universidad de Antioquia, Sistemas Embebidos
 * Curso: Sistemas IoT  
 * Período: 2025-I
 */

#include "mqtt_client.h"
#include "esp_log.h"
#include <cJSON.h>
#include <string.h>
#include <esp_system.h>
#include <esp_event.h>

static const char *TAG = "MQTT_CLIENT";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch (event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Conectado al broker");
        mqtt_connected = true;
        
        // Subscribirse a topic de comandos
        esp_mqtt_client_subscribe(client, "esp32/dht11/commands", 0);
        
        // Publicar estado online
        esp_mqtt_client_publish(client, MQTT_TOPIC_STATUS, "online", 0, 1, 1);
        break;
        
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT Desconectado");
        mqtt_connected = false;
        break;
        
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT Suscrito, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT Desuscrito, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT Mensaje publicado, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT Datos recibidos");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        
        // Procesar comando recibido
        char topic[128], data[256];
        strncpy(topic, event->topic, event->topic_len);
        strncpy(data, event->data, event->data_len);
        mqtt_process_command(topic, data);
        break;
        
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT Error");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Error de transporte: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Error tls stack: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Error captured errno: %d (%s)", event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
        
    default:
        ESP_LOGI(TAG, "Evento MQTT no manejado id:%d", event_id);
        break;
    }
}

esp_err_t mqtt_client_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = "mqtt://" MQTT_BROKER_HOST,
            .address.port = MQTT_BROKER_PORT,
        },
        .credentials = {
            .client_id = MQTT_CLIENT_ID,
            .username = MQTT_USERNAME,
            .authentication.password = MQTT_PASSWORD,
        },
        .session = {
            .last_will = {
                .topic = MQTT_TOPIC_STATUS,
                .msg = "offline",
                .qos = 1,
                .retain = 1,
            }
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Error al inicializar cliente MQTT");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
    
    ESP_LOGI(TAG, "Cliente MQTT inicializado correctamente");
    return ESP_OK;
}

esp_err_t mqtt_publish_sensor_data(sensor_data_t *data) {
    if (!mqtt_connected || mqtt_client == NULL) {
        ESP_LOGW(TAG, "MQTT no conectado, no se puede publicar");
        return ESP_FAIL;
    }

    // Crear JSON con datos del sensor
    cJSON *json = cJSON_CreateObject();
    cJSON *timestamp = cJSON_CreateNumber(data->timestamp);
    cJSON *temperature = cJSON_CreateNumber(data->temperature);
    cJSON *humidity = cJSON_CreateNumber(data->humidity);
    cJSON *device_id = cJSON_CreateString(MQTT_CLIENT_ID);

    cJSON_AddItemToObject(json, "timestamp", timestamp);
    cJSON_AddItemToObject(json, "temperature", temperature);
    cJSON_AddItemToObject(json, "humidity", humidity);
    cJSON_AddItemToObject(json, "device_id", device_id);

    char *json_string = cJSON_Print(json);
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_DATA, 
                                        json_string, 0, 1, 0);
    
    if (msg_id == -1) {
        ESP_LOGE(TAG, "Error al publicar datos del sensor");
        free(json_string);
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Datos del sensor publicados exitosamente (msg_id: %d)", msg_id);
    
    free(json_string);
    cJSON_Delete(json);
    return ESP_OK;
}

esp_err_t mqtt_publish_status(const char *status) {
    if (!mqtt_connected || mqtt_client == NULL) {
        ESP_LOGW(TAG, "MQTT no conectado, no se puede publicar estado");
        return ESP_FAIL;
    }

    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_STATUS, 
                                        status, 0, 1, 1);
    
    if (msg_id == -1) {
        ESP_LOGE(TAG, "Error al publicar estado");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Estado publicado: %s (msg_id: %d)", status, msg_id);
    return ESP_OK;
}

void mqtt_process_command(const char *topic, const char *data) {
    ESP_LOGI(TAG, "Procesando comando: %s -> %s", topic, data);
    
    if (strstr(topic, "commands") != NULL) {
        if (strcmp(data, "read_sensor") == 0) {
            ESP_LOGI(TAG, "Comando: Lectura inmediata del sensor solicitada");
            // Aquí se podría activar una lectura inmediata del sensor
        } else if (strcmp(data, "restart") == 0) {
            ESP_LOGI(TAG, "Comando: Reinicio del sistema solicitado");
            // Esperar un poco antes de reiniciar
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        } else {
            ESP_LOGW(TAG, "Comando no reconocido: %s", data);
        }
    }
}

bool mqtt_is_connected(void) {
    return mqtt_connected;
}

void mqtt_client_deinit(void) {
    if (mqtt_client != NULL) {
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        mqtt_connected = false;
        ESP_LOGI(TAG, "Cliente MQTT deinicializado");
    }
}
