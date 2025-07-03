#ifndef CONFIG_H
#define CONFIG_H

//#define ESP32S3
//#define DISABLE_WIFI  // Отключаем весь WiFi функционал

#ifndef ESP32S3
//#define PIN_VDIV1 14         // Voltage divider #1 (ADC1_CH4)
#define PIN_VDIV1 35
//#define PIN_VDIV2 26         // Voltage divider #2 (ADC1_CH5)
#define PIN_VDIV2 32
#define PIN_CURRENT 34       // ACS712 (ADC1_CH6)
#define PIN_TEMP_AMBIENT 33  // NTC 1
//#define PIN_TEMP_DC 15       // NTC 2 (requires ADC1_CH0 → GPIO36)
//#define PIN_TEMP_NEG 4       // NTC 3
#define PIN_TEMP_NEG 36
//#define PIN_TEMP_POS 12
#define PIN_TEMP_POS 39
#define PIN_RELAY1 25  // Relay 1 (HIGH = ON)
#define PIN_RELAY2 18
#define PIN_LED_STRIP 22

#else
#define PIN_VDIV1 12         // Voltage divider #1 (ADC1_CH4)
#define PIN_VDIV2 4          // Voltage divider #2 (ADC1_CH5)
#define PIN_CURRENT 8        // ACS712 (ADC1_CH6)
#define PIN_TEMP_AMBIENT 10  // NTC 1
#define PIN_TEMP_DC 1        // NTC 2 (requires ADC1_CH0 → GPIO36)
#define PIN_TEMP_NEG 3       // NTC 3
#define PIN_TEMP_POS 2
#define PIN_RELAY1 10  // Relay 1 (HIGH = ON)
#define PIN_RELAY2 7
#define PIN_LED_STRIP 11
#endif

#define LED_COUNT 8

/* ---------- ADC calibration ---------- */
#define ADC_REF_V 3.278f
#define ADC_FULL_SCALE 4095.0f
#define VDIV_RATIO 0.090909f  // 100 k / (1.1 M + 100 k) ≈ 1/11
#define CURRENT_KA 0.026f     // 26 mV/A for ACS712‑20 A
#define CURRENT_VREF (ADC_REF_V / 2)
  // ADC1_CH0 (GPIO36)
static constexpr float VREF = 3.3f;            // фактическое Vref ESP32
static constexpr float VCC = 3.3f;             // питание датчика
static constexpr float V_OFFSET = VCC / 2.0f;  // 1.65 В
static constexpr float SENSITIVITY = 0.0264f;  // В/А (26.4 mV/A)

/* ---------- Network endpoints ---------- */
#define ENDPOINT_BASE \
    "https://t1staging.xtendrobotics.com/api/power/station/activate/"
#define MQTT_SERVER "a1b2c3d4e5f6-ats.iot.eu-central-1.amazonaws.com"

/* ---------- MQTT ---------- */
#define MQTT_PORT_TLS 8883
#define MQTT_PUB_QOS 1
#define MQTT_KEEPALIVE 60

/* ---------- OTA ---------- */
#define ENABLE_OTA 0  // 1 - enable OTA in NetworkManager
#endif
