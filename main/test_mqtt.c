#include <mqtt_client.h>
#include "esp_log.h"

static const char *TAG = "TEST";

void test_mqtt(void) {
    esp_mqtt_client_handle_t client = NULL;
    ESP_LOGI(TAG, "Test MQTT - client handle: %p", client);
}
