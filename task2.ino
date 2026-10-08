#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// =====================================================
// Wi-Fi credentials
// =====================================================
#define WIFI_SSID     "protosem"
#define WIFI_PASSWORD "Proto#123"

// =====================================================
// Adafruit IO credentials
// =====================================================
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "sudhiksha"
#define AIO_KEY         "aio_vYLz34Z5c8sEfr7fSV3n81idu6Of"

// =====================================================
// Relay
// =====================================================
#define RELAY_PIN 26

// =====================================================
// MQTT setup
// =====================================================
WiFiClient client;

Adafruit_MQTT_Client mqtt(
  &client,
  AIO_SERVER,
  AIO_SERVERPORT,
  AIO_USERNAME,
  AIO_KEY
);

// =====================================================
// MQTT feed
// Change "bulb" if your Adafruit IO feed has another name
// =====================================================
Adafruit_MQTT_Subscribe bulb = Adafruit_MQTT_Subscribe(
  &mqtt,
  AIO_USERNAME "/feeds/bulb"
);


// =====================================================
// Connect to Wi-Fi
// =====================================================
void connectWiFi() {

  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


// =====================================================
// Connect to MQTT
// =====================================================
void connectMQTT() {

  Serial.print("Connecting to Adafruit IO MQTT...");

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    Serial.println(mqtt.connectErrorString(ret));

    mqtt.disconnect();

    delay(5000);

    Serial.println("Retrying MQTT connection...");
  }

  Serial.println();
  Serial.println("MQTT connected!");
}


// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 MQTT BULB CONTROLLER");
  Serial.println("================================");

  // ---------------------------------------------------
  // Relay setup
  // Active-LOW relay:
  // LOW  = ON
  // HIGH = OFF
  // ---------------------------------------------------
  pinMode(RELAY_PIN, OUTPUT);

  // Keep bulb OFF when ESP32 starts
  digitalWrite(RELAY_PIN, HIGH);

  Serial.println("Relay initialized: OFF");

  // ---------------------------------------------------
  // Connect Wi-Fi
  // ---------------------------------------------------
  connectWiFi();

  // ---------------------------------------------------
  // Subscribe to MQTT feed
  // ---------------------------------------------------
  mqtt.subscribe(&bulb);

  // ---------------------------------------------------
  // Connect MQTT
  // ---------------------------------------------------
  connectMQTT();
}


// =====================================================
// LOOP
// =====================================================
void loop() {

  // ---------------------------------------------------
  // Reconnect MQTT if connection is lost
  // ---------------------------------------------------
  if (!mqtt.connected()) {
    connectMQTT();
  }

  // ---------------------------------------------------
  // Read MQTT messages
  // ---------------------------------------------------
  Adafruit_MQTT_Subscribe *subscription;

  while ((subscription = mqtt.readSubscription(1000)) != NULL) {

    // Check if message came from the bulb feed
    if (subscription == &bulb) {

      String value = (char *)bulb.lastread;

      // Remove unwanted spaces/newlines
      value.trim();

      // Convert to uppercase
      value.toUpperCase();

      Serial.print("Received: ");
      Serial.println(value);

      // -------------------------------------------------
      // BULB ON
      // Active-LOW relay -> LOW turns relay ON
      // -------------------------------------------------
      if (value == "ON") {

        digitalWrite(RELAY_PIN, LOW);

        Serial.println("GPIO 23 = LOW -> BULB ON");
      }

      // -------------------------------------------------
      // BULB OFF
      // Active-LOW relay -> HIGH turns relay OFF
      // -------------------------------------------------
      else if (value == "OFF") {

        digitalWrite(RELAY_PIN, HIGH);

        Serial.println("GPIO 23 = HIGH -> BULB OFF");
      }

      // -------------------------------------------------
      // Invalid command
      // -------------------------------------------------
      else {

        Serial.println("Unknown command. Use ON or OFF.");
      }
    }
  }
}