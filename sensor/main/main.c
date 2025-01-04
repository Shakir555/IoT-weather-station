#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "dht.h"

#define DHT_GPIO GPIO_NUM_4 // Replace with the GPIO connected to your DHT sensor
#define SENSOR_TYPE DHT_TYPE_DHT11 // Use DHT_TYPE_DHT22 or DHT_TYPE_SI7021 as needed

static const char *TAG = "DHT_MAIN";

void dht_task(void *pvParameter)
{
    while (1)
    {
        float temperature = 0.0;
        float humidity = 0.0;

        esp_err_t result = dht_read_float_data(SENSOR_TYPE, DHT_GPIO, &humidity, &temperature);
        if (result == ESP_OK)
        {
            ESP_LOGI(TAG, "Humidity: %.1f%% Temperature: %.1f°C", humidity, temperature);
        }
        else
        {
            ESP_LOGE(TAG, "Could not read data from sensor: %s", esp_err_to_name(result));
        }

        // Wait for 2 seconds before the next read
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main()
{
    // Initialize GPIO for DHT sensor
    gpio_set_pull_mode(DHT_GPIO, GPIO_PULLUP_ONLY); // Enable pull-up resistor

    // Start the DHT task
    xTaskCreate(&dht_task, "dht_task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}
