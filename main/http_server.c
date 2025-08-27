/*
 * Implementación del Servidor HTTP con Archivos Estáticos
 * 
 * Autor: Edward Fabian Goyeneche Velandia
 * Universidad Nacional de Colombia - Sede Manizales
 * Grupo Percepción de Control Inteligente
 * Asignatura: Desarrollo de Sistemas IoT
 * Período: 2025-II
 * 
 * Descripción: Servidor HTTP que sirve archivos estáticos desde SPIFFS
 * y proporciona API REST para monitoreo del sensor DHT11.
 */

#include "http_server.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include "sensor_manager.h"
#include <string.h>
#include <sys/stat.h>

static const char *TAG = "HTTP_SERVER";

static httpd_handle_t server = NULL;

// Inicializar SPIFFS
static esp_err_t init_spiffs(void) {
    ESP_LOGI(TAG, "Inicializando SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Error montando sistema de archivos");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Partición SPIFFS no encontrada");
        } else {
            ESP_LOGE(TAG, "Error inicializando SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info("storage", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error obteniendo información SPIFFS (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS: %d KB total, %d KB usados", total / 1024, used / 1024);
    }
    
    return ESP_OK;
}

// Obtener tipo MIME basado en extensión
static const char* get_mime_type(const char* filename) {
    if (strstr(filename, ".html")) return "text/html";
    if (strstr(filename, ".css")) return "text/css";
    if (strstr(filename, ".js")) return "application/javascript";
    if (strstr(filename, ".json")) return "application/json";
    if (strstr(filename, ".png")) return "image/png";
    if (strstr(filename, ".jpg") || strstr(filename, ".jpeg")) return "image/jpeg";
    if (strstr(filename, ".ico")) return "image/x-icon";
    return "text/plain";
}

// Handler para archivos estáticos
static esp_err_t static_file_handler(httpd_req_t *req) {
    char filepath[1024];
    const char *filename = req->uri;
    
    // Redirigir root a index.html
    if (strcmp(filename, "/") == 0) {
        filename = "/index.html";
    }
    
    snprintf(filepath, sizeof(filepath), "/spiffs%s", filename);
    
    ESP_LOGI(TAG, "Sirviendo archivo: %s", filepath);
    
    // Verificar si el archivo existe
    struct stat file_stat;
    if (stat(filepath, &file_stat) != 0) {
        ESP_LOGW(TAG, "Archivo no encontrado: %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Archivo no encontrado");
        return ESP_FAIL;
    }
    
    // Abrir archivo
    FILE *file = fopen(filepath, "r");
    if (!file) {
        ESP_LOGE(TAG, "Error abriendo archivo: %s", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error interno del servidor");
        return ESP_FAIL;
    }
    
    // Configurar headers
    httpd_resp_set_type(req, get_mime_type(filename));
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=31536000");
    
    // Enviar archivo en chunks
    char buffer[1024];
    size_t read_bytes;
    
    do {
        read_bytes = fread(buffer, 1, sizeof(buffer), file);
        if (read_bytes > 0) {
            if (httpd_resp_send_chunk(req, buffer, read_bytes) != ESP_OK) {
                fclose(file);
                ESP_LOGE(TAG, "Error enviando chunk");
                return ESP_FAIL;
            }
        }
    } while (read_bytes == sizeof(buffer));
    
    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0); // Terminar envío
    
    return ESP_OK;
}

// Handler para API de datos del sensor
static esp_err_t sensor_api_handler(httpd_req_t *req) {
    sensor_data_t *data = sensor_get_current_data();
    char *json_response = sensor_data_to_json(data);
    
    if (json_response == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error generando JSON");
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
    
    free(json_response);
    return ESP_OK;
}

// Handler para API de estado
static esp_err_t status_api_handler(httpd_req_t *req) {
    cJSON *json = cJSON_CreateObject();
    cJSON *status = cJSON_CreateString("online");
    cJSON *uptime = cJSON_CreateNumber(esp_timer_get_time() / 1000000); // segundos
    cJSON *free_heap = cJSON_CreateNumber(esp_get_free_heap_size());
    cJSON *device_id = cJSON_CreateString("ESP32_C6_DHT11");
    cJSON *author = cJSON_CreateString("Edward Fabian Goyeneche Velandia");
    cJSON *university = cJSON_CreateString("Universidad Nacional de Colombia - Sede Manizales");
    cJSON *course = cJSON_CreateString("Desarrollo de Sistemas IoT - 2025-II");
    
    cJSON_AddItemToObject(json, "status", status);
    cJSON_AddItemToObject(json, "uptime_seconds", uptime);
    cJSON_AddItemToObject(json, "free_heap_bytes", free_heap);
    cJSON_AddItemToObject(json, "device_id", device_id);
    cJSON_AddItemToObject(json, "author", author);
    cJSON_AddItemToObject(json, "university", university);
    cJSON_AddItemToObject(json, "course", course);
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    
    free(json_string);
    return ESP_OK;
}

// Handler para envío manual a AWS
static esp_err_t aws_send_handler(httpd_req_t *req) {
    sensor_data_t *data = sensor_get_current_data();
    
    esp_err_t result = http_send_to_aws_ecs(data);
    
    cJSON *json = cJSON_CreateObject();
    if (result == ESP_OK) {
        cJSON_AddStringToObject(json, "status", "success");
        cJSON_AddStringToObject(json, "message", "Datos DHT11 enviados a EC2 exitosamente");
    } else {
        cJSON_AddStringToObject(json, "status", "error");
        cJSON_AddStringToObject(json, "message", "Error enviando datos DHT11 a EC2");
    }
    
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    
    free(json_string);
    return ESP_OK;
}

esp_err_t http_server_init(void) {
    ESP_LOGI(TAG, "Inicializando servidor HTTP con archivos estáticos");
    
    // Inicializar SPIFFS primero
    esp_err_t ret = init_spiffs();
    if (ret != ESP_OK) {
        return ret;
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = HTTP_SERVER_PORT;
    config.max_uri_handlers = 15;
    config.max_resp_headers = 8;
    config.stack_size = 8192;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Servidor HTTP iniciado en puerto %d", config.server_port);
        
        // Handler para archivos estáticos (debe ser el último para catch-all)
        httpd_uri_t static_uri = {
            .uri = "/*",
            .method = HTTP_GET,
            .handler = static_file_handler,
            .user_ctx = NULL
        };
        
        // Handlers para API (específicos primero)
        httpd_uri_t sensor_api_uri = {
            .uri = "/api/sensor",
            .method = HTTP_GET,
            .handler = sensor_api_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &sensor_api_uri);
        
        httpd_uri_t status_api_uri = {
            .uri = "/api/status",
            .method = HTTP_GET,
            .handler = status_api_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &status_api_uri);
        
        httpd_uri_t aws_send_uri = {
            .uri = "/api/aws-send",
            .method = HTTP_POST,
            .handler = aws_send_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &aws_send_uri);
        
        // Handler estático al final (catch-all)
        httpd_register_uri_handler(server, &static_uri);
        
        ESP_LOGI(TAG, "Handlers HTTP registrados");
        ESP_LOGI(TAG, "Archivos web servidos desde SPIFFS");
        
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Error iniciando servidor HTTP");
    return ESP_FAIL;
}

esp_err_t http_send_to_aws_ecs(sensor_data_t *data) {
    ESP_LOGI(TAG, "Enviando datos DHT11 a AWS EC2...");
    
    // Configurar cliente HTTP para enviar datos a EC2
    esp_http_client_config_t config = {
        .url = AWS_EC2_ENDPOINT "/api/dht11-data", // URL de tu servicio en EC2
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Error inicializando cliente HTTP");
        return ESP_FAIL;
    }
    
    // Preparar datos JSON
    char *json_data = sensor_data_to_json(data);
    if (json_data == NULL) {
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // Configurar headers
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "User-Agent", "ESP32-C6-DHT11/1.0");
    esp_http_client_set_header(client, "X-Device-ID", "ESP32_C6_DHT11");
    
    // Enviar datos
    esp_http_client_set_post_field(client, json_data, strlen(json_data));
    
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Datos DHT11 enviados a EC2, código de respuesta: %d", status_code);
        
        if (status_code >= 200 && status_code < 300) {
            err = ESP_OK;
        } else {
            err = ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Error enviando datos a EC2: %s", esp_err_to_name(err));
    }
    
    free(json_data);
    esp_http_client_cleanup(client);
    
    return err;
}

void http_server_deinit(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Servidor HTTP detenido");
    }
    
    // Deinicializar SPIFFS
    esp_vfs_spiffs_unregister("storage");
    ESP_LOGI(TAG, "SPIFFS deinicializado");
}
