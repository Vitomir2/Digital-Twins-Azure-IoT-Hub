#include <Arduino.h>

// C99 libraries
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <cstdlib>

// Libraries for MQTT client, WiFi connection and SAS-token generation.
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoBearSSL.h>

// Additional sample headers 
#include "secrets.h"

// Utility macros and defines
#define LED_PIN 2

// Translate iot_configs.h defines into variables used by the sample
static const char* ssid = SECRET_WIFI_SSID;
static const char* password = SECRET_WIFI_PASS;
static const char* broker = SECRET_BROKER;
static const String device_id = SECRET_DEVICE_ID;


// Memory allocated for the sample's variables and structures.
static WiFiClient wifi_client; // Used for the TCP socket connection
static BearSSLClient ssl_client(wifi_client); // Used for SSL/TLS connection, integrates with ECC508
static MqttClient mqtt_client(ssl_client);

// Auxiliary functions
unsigned long getTime() {
  // get the current time from the WiFi module-
  return WiFi.getTime();
}
  
static void connectToWiFi() {
  Serial.print("Connecting to WIFI SSID ");
  Serial.println(ssid);

  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

/*
 * Establishses connection with the MQTT Broker (IoT Hub)
 * Some errors you may receive:
 * -- (-.2) Either a connectivity error or an error in the url of the broker
 * -- (-.5) Check credentials - has the SAS Token expired? Do you have the right connection string copied into arduino_secrets?
 */
static void connectToMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  br_x509_certificate x509cert = {
    CLIENT_CERT,
    sizeof(CLIENT_CERT) - 1
  };
  
  // Set the X509 certificate
  ssl_client.setEccCert(x509cert);

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the username to "<broker>/<device id>/?api-version=2018-06-30"
  String username;

  // Set the client id used for MQTT as the device id
  mqtt_client.setId(device_id);

  username += broker;
  username += "/";
  username += device_id;
  username += "/api-version=2020-09-30";
  // this sketch - Arduino-IoT-Hub-Temperature.azure-devices.net/temperature-sensor-1/api-version=2020-09-30
  // symmetric key sketch - Arduino-IoT-Hub-Temperature.azure-devices.net/temp-sensor-1/?api-version=2020-09-30&DeviceClientType=c%2F1.3.2(ard;esp8266)
  Serial.print("Username: ");
  Serial.println(username);
  mqtt_client.setUsernamePassword(username, "");

  while (!mqtt_client.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    // Serial.println(mqtt_client.connectError());
    delay(500);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();
  
  // subscribe to a topic
  mqtt_client.subscribe("devices/" + device_id + "/messages/devicebound/#");
}

static void establishConnection() {
  if(WiFi.status() != WL_CONNECTED) {
    // if WiFi is disconnected => connect
    connectToWiFi();
  }

  if (!mqtt_client.connected()) {
    // if MQTT client is disconnected => connect
    connectToMQTT();
  }
}

// Arduino setup and loop main functions.
void setup() {
  Serial.begin(115200);

  establishConnection();
}

void loop() {
  establishConnection();
}
