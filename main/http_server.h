/*
 * Servidor HTTP para Interfaz Web y API REST
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_err.h"
#include "sensor_manager.h"
#include "config.h"

/**
 * @brief Inicializa el servidor HTTP
 * @return ESP_OK si la inicialización fue exitosa
 */
esp_err_t http_server_init(void);

/**
 * @brief Envía datos del sensor a AWS ECS via HTTP
 * @param data Puntero a los datos del sensor
 * @return ESP_OK si el envío fue exitoso
 */
esp_err_t http_send_to_aws_ecs(sensor_data_t *data);

/**
 * @brief Deinicializa el servidor HTTP
 */
void http_server_deinit(void);

#endif // HTTP_SERVER_H
