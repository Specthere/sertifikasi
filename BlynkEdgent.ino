
/************************************************************
 * PROJECT : BLUINO ESP32 IOT MONITORING
 * HARDWARE: Bluino ESP32 IoT StarterKit Shield V3.2
 * PLATFORM: Blynk IoT + Blynk.Air OTA
 *
 * SENSOR:
 * - DHT11  : Temperature & Humidity
 * - BMP180 : Atmospheric Pressure
 *
 * ACTUATOR:
 * - Red LED
 * - Yellow LED
 * - Green LED
 * - Blue LED
 ************************************************************/

// ==========================================================
// BLYNK CONFIGURATION
// ==========================================================

#define BLYNK_TEMPLATE_ID "TMPL68lRAvzpS"
#define BLYNK_TEMPLATE_NAME "IoT Monitoring"

#define BLYNK_FIRMWARE_VERSION "1.0.3"

#define BLYNK_PRINT Serial
#define APP_DEBUG

// ESP32 DEVKIT V1
#define USE_ESP32_DEV_MODULE
#include "BlynkEdgent.h"
#include "BoardConfig.h"
#include "Interaction.h"

#include <WiFi.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

// ==========================================================
// LIBRARY
// ==========================================================

#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

// ==========================================================
// PIN CONFIGURATION
// ==========================================================

// DHT11
#define DHT_PIN 27
#define DHT_TYPE DHT11

// BMP180 I2C
#define I2C_SDA 21
#define I2C_SCL 22

// LED OUTPUT
#define LED_RED    4
#define LED_YELLOW 17
#define LED_GREEN  2
#define LED_BLUE   16

// ==========================================================
// SENSOR OBJECT
// ==========================================================

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_BMP085 bmp;

// ==========================================================
// TIMER
// ==========================================================

BlynkTimer sensorTimer;

// ==========================================================
// SENSOR STATUS
// ==========================================================

bool bmpReady = false;

// ==========================================================
// LED CONTROL FUNCTION
// ==========================================================

void controlLED(int pin, int state)
{
  digitalWrite(pin, state ? HIGH : LOW);
}

// ==========================================================
// BLYNK LED CONTROL
// ==========================================================

// RED LED - V3
BLYNK_WRITE(V3)
{
  int state = param.asInt();

  controlLED(LED_RED, state);

  Serial.print("Red LED: ");
  Serial.println(state ? "ON" : "OFF");
}

// YELLOW LED - V4
BLYNK_WRITE(V4)
{
  int state = param.asInt();

  controlLED(LED_YELLOW, state);

  Serial.print("Yellow LED: ");
  Serial.println(state ? "ON" : "OFF");
}

// GREEN LED - V5
BLYNK_WRITE(V5)
{
  int state = param.asInt();

  controlLED(LED_GREEN, state);

  Serial.print("Green LED: ");
  Serial.println(state ? "ON" : "OFF");
}

// BLUE LED - V6
BLYNK_WRITE(V6)
{
  int state = param.asInt();

  controlLED(LED_BLUE, state);

  Serial.print("Blue LED: ");
  Serial.println(state ? "ON" : "OFF");
}

// ==========================================================
// BLYNK CONNECTION CALLBACK
// ==========================================================

BLYNK_CONNECTED()
{
  Serial.println("Connected to Blynk Cloud");

  // Synchronize LED switch values
  Blynk.syncVirtual(V3, V4, V5, V6);
}

// ==========================================================
// READ AND SEND SENSOR DATA
// ==========================================================

void sendSensorData()
{
  if (!Blynk.connected())
  {
    Serial.println("Blynk belum terhubung. Data tidak dikirim.");
    return;
  }
  // Read DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read BMP180
  float pressure = -1;

  if (bmpReady)
  {
    pressure = bmp.readPressure() / 100.0;
  }

  // --------------------------------------------------------
  // TEMPERATURE
  // --------------------------------------------------------

  if (!isnan(temperature))
  {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Blynk.virtualWrite(V0, temperature);
  }
  else
  {
    Serial.println("ERROR: DHT11 temperature read failed");
  }

  // --------------------------------------------------------
  // HUMIDITY
  // --------------------------------------------------------

  if (!isnan(humidity))
  {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Blynk.virtualWrite(V1, humidity);
  }
  else
  {
    Serial.println("ERROR: DHT11 humidity read failed");
  }

  // --------------------------------------------------------
  // ATMOSPHERIC PRESSURE
  // --------------------------------------------------------

  if (bmpReady && pressure > 0)
  {
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");

    Blynk.virtualWrite(V2, pressure);
  }
  else
  {
    Serial.println("ERROR: BMP180 read failed");
  }

  Serial.println("-----------------------------");
}

// ==========================================================
// SETUP
// ==========================================================

void setup()
{
  Serial.begin(115200);
  Serial.println("OTA TEST BERHASIL - FIRMWARE 1.0.1");

  delay(500);

  Serial.println();
  Serial.println("================================");
  Serial.println(" BLUINO ESP32 IOT MONITORING");
  Serial.println("================================");

  // --------------------------------------------------------
  // LED INITIALIZATION
  // --------------------------------------------------------

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // All LEDs OFF at startup
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);

  // --------------------------------------------------------
  // I2C INITIALIZATION
  // --------------------------------------------------------

  Wire.begin(I2C_SDA, I2C_SCL);

  // --------------------------------------------------------
  // DHT11 INITIALIZATION
  // --------------------------------------------------------

  dht.begin();

  Serial.println("DHT11 initialized");

  // --------------------------------------------------------
  // BMP180 INITIALIZATION
  // --------------------------------------------------------

  if (bmp.begin())
  {
    bmpReady = true;

    Serial.println("BMP180 initialized successfully");
  }
  else
  {
    bmpReady = false;

    Serial.println("ERROR: BMP180 not detected!");
  }

  // Inisialisasi Interaction
  interaction.begin();
  // --------------------------------------------------------
  // BLYNK EDGENT INITIALIZATION
  // --------------------------------------------------------

  Serial.println("Starting Blynk.Edgent...");

  BlynkEdgent.begin();

  // --------------------------------------------------------
  // SENSOR TIMER
  // --------------------------------------------------------

  sensorTimer.setInterval(2000L, sendSensorData);

  Serial.println("System initialization completed");
}

// ==========================================================
// MAIN LOOP
// ==========================================================

void loop()
{
  BlynkEdgent.run();
  interaction.run();
  sensorTimer.run();

  static unsigned long lastCheck = 0;

  if (millis() - lastCheck >= 3000)
  {
    lastCheck = millis();

    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Blynk Status: ");
    Serial.println(Blynk.connected() ? "CONNECTED" : "DISCONNECTED");
  }

  delay(1);
}
