/*
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 */

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// MQTT Configuration - Amazon EC2 Mosquitto
#define MQTT_BROKER_HOST "ec2-13-220-220-130.compute-1.amazonaws.com"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "esp32_devkit_c6_001"
#define MQTT_USERNAME "esp32_user"     // Usuario MQTT (configura en EC2)
#define MQTT_PASSWORD "esp32_pass"     // Password MQTT (configura en EC2)
#define MQTT_TOPIC_SENSOR_DATA "esp32/dht11/data"
#define MQTT_TOPIC_STATUS "esp32/dht11/status"

// HTTP Server Configuration
#define HTTP_SERVER_PORT 80

// Amazon ECS/EC2 Configuration
#define AWS_EC2_ENDPOINT "http://ec2-13-220-220-130.compute-1.amazonaws.com:8080"
#define WEBSOCKET_PORT 9001

// DHT11 Sensor Configuration
#define SENSOR_READ_INTERVAL_MS 2000  // 2 segundos 
#define DHT11_GPIO_PIN GPIO_NUM_4     // GPIO4 para DHT11
#define DHT11_SENSOR_TYPE DHT_TYPE_DHT11

// Buffer sizes
#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_MQTT_MESSAGE_SIZE 512

#endif // CONFIG_H
