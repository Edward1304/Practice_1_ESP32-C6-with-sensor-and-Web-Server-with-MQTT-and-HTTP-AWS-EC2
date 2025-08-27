/*
 * Gestor de Sensor DHT11 para ESP32-C6
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>
#include "config.h"

typedef struct {
    uint64_t timestamp;     // Timestamp en milisegundos
    float temperature;      // Temperatura en °C (DHT11)
    float humidity;         // Humedad en % (DHT11)
    bool sensor_valid;      // Indica si la lectura es válida
    int error_count;        // Contador de errores de lectura
} sensor_data_t;

// Callback para notificar cuando hay nuevos datos
typedef void (*sensor_data_callback_t)(sensor_data_t *data);

/**
 * @brief Inicializa el sensor DHT11
 * @return ESP_OK si la inicialización fue exitosa
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Task para leer DHT11 periódicamente
 */
void sensor_read_task(void *pvParameters);

/**
 * @brief Lee datos del DHT11 una vez
 * @param data Puntero para almacenar los datos leídos
 * @return ESP_OK si la lectura fue exitosa
 */
esp_err_t sensor_read_dht11(sensor_data_t *data);

/**
 * @brief Obtiene los datos actuales del sensor
 * @return Puntero a la estructura con los datos del sensor
 */
sensor_data_t* sensor_get_current_data(void);

/**
 * @brief Registra un callback para ser notificado de nuevos datos
 * @param callback Función callback a registrar
 */
void sensor_register_callback(sensor_data_callback_t callback);

/**
 * @brief Convierte los datos del sensor a formato JSON
 * @param data Puntero a los datos del sensor
 * @return String JSON (debe ser liberado con free())
 */
char* sensor_data_to_json(sensor_data_t *data);

/**
 * @brief Deinicializa el gestor de sensores
 */
void sensor_manager_deinit(void);

#endif // SENSOR_MANAGER_H
