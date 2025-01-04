// Libraries
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_http_client.h>
#include <dht.h>

// Sensor Definition
#define DHT_PIN GPIO_NUM_4
#define MAX_HTTP_OUTPUT_BUFF 512

// Send Temperature and Humidity Data to Django Server
static void send_data_to_server(float temperature, float humidity)
{
    char post_data[100];
    snprintf(post_data, sizeof(post_data),
             "{\"temperature\": %.2f, \"humidity\": %.2f}",
             temperature, humidity);

    // Correctly define and initialize esp_http_client_config_t
    esp_http_client_config_t config = {
        .url = "http://<YOUR_DJANGO_SERVER_IP>:8000/api/sensor-data/",
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        printf("HTTP POST Status = %d\n",
               esp_http_client_get_status_code(client));
    }
    else
    {
        printf("HTTP POST request failed: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

// DHT Task
void dht_task(void *pvParam)
{
    float temperature = 0;
    float humidity = 0;

    while (1) // Loop for periodic sensor reading
    {
        if (dht_read_float_data(DHT_TYPE_DHT11, DHT_PIN, &humidity, &temperature) == ESP_OK)
        {
            printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
            send_data_to_server(temperature, humidity);
        }
        else
        {
            printf("Failed to read data from DHT sensor\n");
        }
        // 30-second delay
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}

//void app_main()
//{
//  xTaskCreate(&dht_task, "dht_task", 2048, NULL, 5, NULL);
//}
