#define ENABLE_SERVO true
#define SERVO1_PIN GPIO_NUM_23
#define SERVO_SERIAL_DEBUG false
#define SERVO_OPEN_TIMEOUT 300 // 0.3 second
#define SERVO_OPEN_ANGLE 135
#define SERVO_CLOSE_ANGLE 180

#define ENABLE_DHT true
#define DHT_PIN GPIO_NUM_19

#define ENABLE_WATER_LEVEL_SENSOR true
#define WATER_LEVEL_SENSOR_PIN GPIO_NUM_36

#define ENABLE_WATER_PUMP true
#define WATER_PUMP_PIN GPIO_NUM_18
#define WATER_PUMP_ENABLE_TIMEOUT 45000 // 45 seconds

#define ENABLE_LCD_OUTPUT true
#define LCD_COLUMNS_SIZE 16
#define LCD_ROWS_SIZE 2
#define LCD_TEMP_FORMAT "T %.1f\xDF""C"
#define LCD_HUMIDITY_FORMAT "H %d%%"
#define LCD_WATER_LEVEL_FORMAT "W %d%%"
#define WIFI_CONNECT_TEXT "Waiting WiFi"
#define WIFI_CONNECT_DOT_MAX 3

#define ENABLE_WIFI true
#define WIFI_SSID "IoT-Kelompok-M2D"
#define WIFI_PASSWORD "12345678"
#define WS_SERVER_HW_ID "petfeeder-esp32dev-example"
#define WS_SERVER_ADDRESS "example.com"
#define WS_SERVER_PORT 8080
#define WS_RTX_ON true // Enable WebSocket Real-Time Exchange (RTX) mode