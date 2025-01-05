//Libraries
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <dht.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>

//Sensor Definition
#define DHT_GPIO 4
#define DHT_TYPE DHT_TYPE_DHT11

//Wifi Definition
#define WIFI_SSID     "Your_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#define SERVER_URL    "http://<your-django-server-ip>/api/sensor-data/"

//Logging
#define TAG = "DHT_HTTP_CLIENT"

//Wifi Station
void wifi_init_sta(void)
{
    //NVS Flash Initialization
    esp_err_t ret = nvs_flash_init();
    
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //Wifi Initialization
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default())
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //Wifi Config
    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "connecting to Wifi...");
}

void send_sensor_data(float temperature, float humidity)
{
    char post_data[100];
    snprintf(post_data, sizeof(post_data), "{\"temperature\": %.2f, \"humidity\": %.2f}", temperature, humidity);

    //HTTP Client Configuration
    esp_http_client_config_t config =
    {
        .url    = SERVER_URL,
        .method = HTTP_METHOD_POST,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP POST Status: %d, Response Length: %d",
                      esp_http_client_get_status_code(client),
                      esp_http_client_get_response_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP POST Failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

//DHT Task
void dht_task(void*pvParam)
{
    while(1)
    {
        float temperature = 0;
        float humidity = 0;

        if (dht_read_float_data(DHT_TYPE, DHT_GPIO, &humidity, &temperature) == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature: %.2f°C, Humidity: %.2f%%",
                          temperature, humidity);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read data from DHT sensor");
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "DHT11 Starting application");

    //Connect to Wifi Station
    wifi_init_sta();
    xTaskCreate(&dht_task, "dht_task", 4096, NULL, 5, NULL);
}
