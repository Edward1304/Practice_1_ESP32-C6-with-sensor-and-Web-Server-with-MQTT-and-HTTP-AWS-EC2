/*
 * Configuración WiFi para ESP32-C6
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "esp_err.h"
#include "esp_netif.h"
#include "config.h"

/**
 * @brief Inicializa y conecta WiFi en modo estación
 * @return ESP_OK si la conexión fue exitosa
 */
esp_err_t wifi_init_sta(void);

/**
 * @brief Verifica si WiFi está conectado
 * @return true si está conectado, false en caso contrario
 */
bool wifi_is_connected(void);

/**
 * @brief Obtiene información de IP del WiFi
 * @param ip_info Estructura para almacenar la información IP
 * @return ESP_OK si fue exitoso
 */
esp_err_t wifi_get_ip_info(esp_netif_ip_info_t *ip_info);

#endif // WIFI_CONFIG_H
