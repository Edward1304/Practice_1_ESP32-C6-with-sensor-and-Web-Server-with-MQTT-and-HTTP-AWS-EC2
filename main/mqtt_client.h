/*
 * Cliente MQTT para Comunicación con Mosquitto en AWS EC2
 * 
 * Autor: Edwar David Valencia Gomez
 * Universidad de Antioquia, Sistemas Embebidos
 * Curso: Sistemas IoT
 * Período: 2025-I
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "esp_err.h"
#include "sensor_manager.h"
#include "config.h"
#include "mqtt_client.h"

/**
 * @brief Inicializa el cliente MQTT
 * @return ESP_OK si la inicialización fue exitosa
 */
esp_err_t mqtt_client_init(void);

/**
 * @brief Publica los datos del sensor via MQTT
 * @param data Puntero a los datos del sensor
 * @return ESP_OK si la publicación fue exitosa
 */
esp_err_t mqtt_publish_sensor_data(sensor_data_t *data);

/**
 * @brief Publica el estado del dispositivo
 * @param status String con el estado
 * @return ESP_OK si la publicación fue exitosa
 */
esp_err_t mqtt_publish_status(const char *status);

/**
 * @brief Procesa comandos recibidos via MQTT
 * @param topic Topic del mensaje
 * @param data Datos del mensaje
 */
void mqtt_process_command(const char *topic, const char *data);

/**
 * @brief Verifica si el cliente MQTT está conectado
 * @return true si está conectado, false en caso contrario
 */
bool mqtt_is_connected(void);

/**
 * @brief Deinicializa el cliente MQTT
 */
void mqtt_client_deinit(void);

#endif // MQTT_CLIENT_H
