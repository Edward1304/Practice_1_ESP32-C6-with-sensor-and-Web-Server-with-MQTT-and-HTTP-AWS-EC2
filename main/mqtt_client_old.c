/*
 * Implementación del Cliente MQTT con Autenticación
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 * 
 * Descripción: Cliente MQTT que se conecta a broker Mosquitto en EC2
 * con autenticación por usuario/password para envío de datos del DHT11.
 */

#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_tls.h"
#include "sensor_manager.h"
#include <string.h>

static const char *TAG = "MQTT_CLIENT";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Conectado al broker");
        mqtt_connected = true;
        
        // Suscribirse a topics de comandos
        esp_mqtt_client_subscribe(client, "esp32/dht11/commands", 0);
        esp_mqtt_client_subscribe(client, "esp32/dht11/config", 0);
        
        // Publicar mensaje de estado
        esp_mqtt_client_publish(client, MQTT_TOPIC_STATUS, "online", 0, 1, 1);
        break;
        
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT Desconectado del broker");
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
        
        // Procesar comandos recibidos
        char topic[64] = {0};
        char data[256] = {0};
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
        ESP_LOGI(TAG, "Evento MQTT no manejado: %ld", event_id);
        break;
    }
}

esp_err_t mqtt_client_init(void) {
    ESP_LOGI(TAG, "Inicializando cliente MQTT con autenticación");
    
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
        }
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Error inicializando cliente MQTT");
        return ESP_FAIL;
    }
    
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
    
    ESP_LOGI(TAG, "Cliente MQTT inicializado con broker: %s:%d", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    return ESP_OK;
}

esp_err_t mqtt_publish_sensor_data(sensor_data_t *data) {
    if (!mqtt_connected || mqtt_client == NULL) {
        ESP_LOGW(TAG, "MQTT no conectado, no se pueden enviar datos");
        return ESP_FAIL;
    }
    
    char *json_data = sensor_data_to_json(data);
    if (json_data == NULL) {
        ESP_LOGE(TAG, "Error creando JSON");
        return ESP_FAIL;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_SENSOR_DATA, json_data, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Error publicando datos del sensor");
        free(json_data);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Datos del sensor publicados, msg_id=%d", msg_id);
    free(json_data);
    return ESP_OK;
}

esp_err_t mqtt_publish_status(const char *status) {
    if (!mqtt_connected || mqtt_client == NULL) {
        ESP_LOGW(TAG, "MQTT no conectado");
        return ESP_FAIL;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_STATUS, status, 0, 1, 1);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Error publicando estado");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Estado publicado: %s", status);
    return ESP_OK;
}

void mqtt_process_command(const char *topic, const char *data) {
    ESP_LOGI(TAG, "Procesando comando - Topic: %s, Data: %s", topic, data);
    
    if (strcmp(topic, "esp32/dht11/commands") == 0) {
        if (strcmp(data, "restart") == 0) {
            ESP_LOGI(TAG, "Comando de reinicio recibido");
            mqtt_publish_status("restarting");
            esp_restart();
        } else if (strcmp(data, "status") == 0) {
            sensor_data_t *sensor_data = sensor_get_current_data();
            mqtt_publish_sensor_data(sensor_data);
        } else if (strcmp(data, "read_sensor") == 0) {
            ESP_LOGI(TAG, "Solicitando lectura manual del DHT11");
            sensor_data_t *sensor_data = sensor_get_current_data();
            mqtt_publish_sensor_data(sensor_data);
        }
    } else if (strcmp(topic, "esp32/dht11/config") == 0) {
        ESP_LOGI(TAG, "Configuración recibida: %s", data);
        // Aquí puedes procesar cambios de configuración específicos del DHT11
        // Por ejemplo, cambiar intervalo de lectura, calibración, etc.
    }
}

bool mqtt_is_connected(void) {
    return mqtt_connected;
}

void mqtt_client_deinit(void) {
    if (mqtt_client) {
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        mqtt_connected = false;
    }
}
