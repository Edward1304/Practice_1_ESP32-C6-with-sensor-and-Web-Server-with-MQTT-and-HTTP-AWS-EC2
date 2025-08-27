/*
 * Implementación del Gestor de Sensor DHT11
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 * 
 * Descripción: Implementa el protocolo de comunicación con el sensor DHT11
 * para lectura de temperatura y humedad relativa.
 */

#include "sensor_manager.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "SENSOR_DHT11";

static sensor_data_t current_sensor_data = {0};
static TaskHandle_t sensor_task_handle = NULL;

// Callback para notificar nuevos datos
static sensor_data_callback_t data_callback = NULL;

// Variables para el protocolo DHT11
#define DHT11_START_SIGNAL_1    18000  // 18ms
#define DHT11_START_SIGNAL_2    80     // 80us
#define DHT11_RESPONSE_SIGNAL   80     // 80us
#define DHT11_BIT_1_SIGNAL      70     // 70us para bit 1
#define DHT11_BIT_0_SIGNAL      26     // 26us para bit 0

// Función para enviar señal de inicio al DHT11
static void dht11_send_start_signal(void) {
    gpio_set_direction(DHT11_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_GPIO_PIN, 0);
    esp_rom_delay_us(DHT11_START_SIGNAL_1);  // 18ms en bajo
    gpio_set_level(DHT11_GPIO_PIN, 1);
    esp_rom_delay_us(40);  // 40us en alto
    gpio_set_direction(DHT11_GPIO_PIN, GPIO_MODE_INPUT);
}

// Función para esperar una señal específica
static bool dht11_wait_for_signal(int level, int timeout_us) {
    int count = 0;
    while (gpio_get_level(DHT11_GPIO_PIN) != level) {
        esp_rom_delay_us(1);
        count++;
        if (count > timeout_us) {
            return false;
        }
    }
    return true;
}

// Función para leer un bit del DHT11
static int dht11_read_bit(void) {
    int count = 0;
    
    // Esperar que la línea vaya a alto (inicio de bit)
    if (!dht11_wait_for_signal(1, 100)) {
        return -1;
    }
    
    // Contar cuánto tiempo está en alto
    while (gpio_get_level(DHT11_GPIO_PIN) == 1) {
        esp_rom_delay_us(1);
        count++;
        if (count > 100) {
            return -1;
        }
    }
    
    // Si está más de 40us en alto, es un '1', sino es un '0'
    return (count > 40) ? 1 : 0;
}

esp_err_t sensor_manager_init(void) {
    ESP_LOGI(TAG, "Inicializando sensor DHT11 en GPIO%d", DHT11_GPIO_PIN);
    
    // Configurar GPIO como pull-up
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << DHT11_GPIO_PIN),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    // Inicializar datos del sensor
    current_sensor_data.sensor_valid = false;
    current_sensor_data.error_count = 0;

    // Crear task para lectura periódica
    xTaskCreate(sensor_read_task, "dht11_task", 4096, NULL, 5, &sensor_task_handle);

    ESP_LOGI(TAG, "Sensor DHT11 inicializado correctamente");
    return ESP_OK;
}

esp_err_t sensor_read_dht11(sensor_data_t *data) {
    uint8_t dht11_data[5] = {0};
    
    // Deshabilitar interrupciones durante la lectura crítica
    taskDISABLE_INTERRUPTS();
    
    // Enviar señal de inicio
    dht11_send_start_signal();
    
    // Esperar respuesta del DHT11
    if (!dht11_wait_for_signal(0, 100)) {  // Esperar señal baja
        taskENABLE_INTERRUPTS();
        ESP_LOGW(TAG, "DHT11 no respondió - señal de inicio");
        return ESP_FAIL;
    }
    
    if (!dht11_wait_for_signal(1, 100)) {  // Esperar señal alta
        taskENABLE_INTERRUPTS();
        ESP_LOGW(TAG, "DHT11 no respondió - señal alta");
        return ESP_FAIL;
    }
    
    if (!dht11_wait_for_signal(0, 100)) {  // Esperar que termine señal alta
        taskENABLE_INTERRUPTS();
        ESP_LOGW(TAG, "DHT11 no respondió - fin señal alta");
        return ESP_FAIL;
    }
    
    // Leer 40 bits de datos
    for (int i = 0; i < 40; i++) {
        int bit = dht11_read_bit();
        if (bit < 0) {
            taskENABLE_INTERRUPTS();
            ESP_LOGW(TAG, "Error leyendo bit %d", i);
            return ESP_FAIL;
        }
        
        dht11_data[i / 8] = (dht11_data[i / 8] << 1) | bit;
    }
    
    taskENABLE_INTERRUPTS();
    
    // Verificar checksum
    uint8_t checksum = dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3];
    if (checksum != dht11_data[4]) {
        ESP_LOGW(TAG, "Checksum incorrecto: calculado=%d, recibido=%d", checksum, dht11_data[4]);
        return ESP_FAIL;
    }
    
    // Extraer datos
    data->humidity = (float)dht11_data[0];  // DHT11 solo da parte entera
    data->temperature = (float)dht11_data[2];  // DHT11 solo da parte entera
    data->timestamp = esp_timer_get_time() / 1000; // ms
    data->sensor_valid = true;
    
    ESP_LOGI(TAG, "DHT11 - Temp: %.1f°C, Hum: %.1f%%", data->temperature, data->humidity);
    return ESP_OK;
}

void sensor_read_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    ESP_LOGI(TAG, "Iniciando task de lectura DHT11");
    
    while (1) {
        sensor_data_t new_data = {0};
        
        esp_err_t result = sensor_read_dht11(&new_data);
        
        if (result == ESP_OK) {
            // Actualizar datos globales
            current_sensor_data = new_data;
            current_sensor_data.error_count = 0;
            
            // Notificar callback si está registrado
            if (data_callback) {
                data_callback(&current_sensor_data);
            }
        } else {
            current_sensor_data.error_count++;
            current_sensor_data.sensor_valid = false;
            ESP_LOGW(TAG, "Error leyendo DHT11 (errores consecutivos: %d)", current_sensor_data.error_count);
            
            // Si hay muchos errores, reiniciar GPIO
            if (current_sensor_data.error_count > 5) {
                ESP_LOGW(TAG, "Demasiados errores, reinicializando GPIO");
                sensor_manager_init();
                current_sensor_data.error_count = 0;
            }
        }

        // DHT11 requiere al menos 1 segundo entre lecturas
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
    }
}

sensor_data_t* sensor_get_current_data(void) {
    return &current_sensor_data;
}

void sensor_register_callback(sensor_data_callback_t callback) {
    data_callback = callback;
}

char* sensor_data_to_json(sensor_data_t *data) {
    cJSON *json = cJSON_CreateObject();
    
    cJSON *timestamp = cJSON_CreateNumber(data->timestamp);
    cJSON *temperature = cJSON_CreateNumber(data->temperature);
    cJSON *humidity = cJSON_CreateNumber(data->humidity);
    cJSON *sensor_valid = cJSON_CreateBool(data->sensor_valid);
    cJSON *error_count = cJSON_CreateNumber(data->error_count);
    cJSON *device_id = cJSON_CreateString("ESP32_C6_DHT11");
    cJSON *sensor_type = cJSON_CreateString("DHT11");
    cJSON *gpio_pin = cJSON_CreateNumber(DHT11_GPIO_PIN);
    
    cJSON_AddItemToObject(json, "timestamp", timestamp);
    cJSON_AddItemToObject(json, "temperature", temperature);
    cJSON_AddItemToObject(json, "humidity", humidity);
    cJSON_AddItemToObject(json, "sensor_valid", sensor_valid);
    cJSON_AddItemToObject(json, "error_count", error_count);
    cJSON_AddItemToObject(json, "device_id", device_id);
    cJSON_AddItemToObject(json, "sensor_type", sensor_type);
    cJSON_AddItemToObject(json, "gpio_pin", gpio_pin);
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    return json_string;
}

void sensor_manager_deinit(void) {
    if (sensor_task_handle) {
        vTaskDelete(sensor_task_handle);
        sensor_task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "Sensor DHT11 deinicializado");
}
