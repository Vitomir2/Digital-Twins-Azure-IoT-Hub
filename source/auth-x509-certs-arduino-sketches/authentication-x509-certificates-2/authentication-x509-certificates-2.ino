/*
   Azure IoT Device Provisioning Service registration tool
   This sketch securely connects to an Azure IoT DPS using MQTT over WiFi,
   secured by SSl.
   It uses a private key stored in the built-in crypto chip and a CA signed
   public certificate for SSL/TLS authetication.
   It subscribes to a DPS topic to receive the response, and publishes a message
   to stard the device enrollment.
   Boards:
   - Arduino Nano 33 IoT
   Author: Nicola Elia
   GNU General Public License v3.0
*/

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <utility/ECCX08SelfSignedCert.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>

#include "secrets.h"

// ================================== SETTINGS ===================================
// Set self_signed_cert to true for using the crypto chip stored certificate
// Set self_signed_cert to false to use an hardcoded certificate
bool self_signed_cert = false;
const int keySlot     = 0;  // Crypto chip slot to pick the key from
const int certSlot    = 8;  // Crypto chip slot to pick the certificate from
// ===============================================================================

const char* ssid       = SECRET_WIFI_SSID;
const char* pass       = SECRET_WIFI_PASS;
const char* broker     = SECRET_BROKER;
const char* DPS_broker = SECRET_DPS_BROKER;
const String idScope   = SECRET_ID_SCOPE;
const String device_id = SECRET_DEVICE_ID;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECCX08
MqttClient    mqttClient(sslClient); // Used for MQTT protocol usage

unsigned long lastMillis = 0;

void setup() {
  // Wait for serial
  Serial.begin(9600);
  while (!Serial);

  // Check the crypto chip module presence (needed for BearSSL)
  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // ================ SSL SETUP ================
  // Set a callback to get the current time
  // (used to validate the servers certificate)
  ArduinoBearSSL.onGetTime(getTime);

  if (self_signed_cert) {
    Serial.println("Loading the self-signed certificate from ECCX08...");
    // Reconstruct the self signed cert
    ECCX08SelfSignedCert.beginReconstruction(keySlot, certSlot);
    // In case of self-signed certificate with ECCX08SelfSignedCert.ino, the ECCX08
    // crypto chip serial number is used as CN by default. Otherwise, change it here.
    ECCX08SelfSignedCert.setCommonName(ECCX08.serialNumber());
    ECCX08SelfSignedCert.endReconstruction();

    // Instruct the SSL client to use the chosen ECCX08 slot for picking the private key
    // and set the reconstructed certificate as accompanying public certificate.
    sslClient.setEccSlot(
      keySlot,
      ECCX08SelfSignedCert.bytes(),
      ECCX08SelfSignedCert.length()
    );
  } else if (!self_signed_cert) {
    Serial.println("Using the certificate from secrets.h...");

    // Instruct the SSL client to use the chosen ECCX08 slot for picking the private key
    // and set the hardcoded certificate as accompanying public certificate.
    sslClient.setEccSlot(
      keySlot,
      CLIENT_PUBLIC_CERT);
    
    // br_x509_certificate x509cert = {
    //  CLIENT_CERT,
    //  sizeof(CLIENT_CERT) - 1
    // };

    // Set the X509 certificate
    // sslClient.setEccCert(x509cert);

    // sslClient.setKey(CLIENT_KEY, CLIENT_CERT);
  }

  /*
     Note: I prefer to use BearSSLClient::setEccSlot because it contains a function to decode
     the .pem certificate (the hardcoded certificate can be stored in base64, instead of
     converting it to binary), and it automatically computes the certificate length.
  */

  // ================ MQTT Client SETUP ================
  // Set the client id used for MQTT as the device_id
  mqttClient.setId(device_id);

  // Set the username to "<idScope>/registrations/<registrationId>/api-version=2019-03-31"
  // String username = broker + "/registrations/" + device_id + "/api-version=2019-03-31";
  String username;
  username += broker;
  username += "/";
  username += device_id;
  username += "/?api-version=2018-06-30";

  // Set an empty password if you want to use X.509 authentication, otherwise set the SAS token connection string
  String password = "";

  // Authenticate the MQTT Client
  mqttClient.setUsernamePassword(username, password);

  // Set the on message callback, called when the MQTT Client receives a message
  mqttClient.onMessage(onMessageReceived);
}

void publishMessage() {
  Serial.println("Publishing message");
  
  const int capacity = JSON_ARRAY_SIZE(10) + 10*JSON_OBJECT_SIZE(2)+ JSON_OBJECT_SIZE(3) + 280;     // Calculation of the JSON doc size, as explained in the documentation

  /*  
  Here is where you should write the body of your message. In this example, the JSON doc is purposely longer than 256 characters, to highlight the issue.
  */

  StaticJsonDocument<capacity> doc;
  doc["topic"] = "messageTopic";  
  doc["deviceId"] = device_id;
  JsonArray data = doc.createNestedArray("data");
  JsonObject data_0 = data.createNestedObject();
  data_0["label"] = "Ankara";
  data_0["state"] = true;
  JsonObject data_1 = data.createNestedObject();
  data_1["label"] = "Beirut";
  data_1["state"] = true;
  JsonObject data_2 = data.createNestedObject();
  data_2["label"] = "Cincinnati";
  data_2["state"] = false;
  JsonObject data_3 = data.createNestedObject();
  data_3["label"] = "Detroit";
  data_3["state"] = false;
  JsonObject data_4 = data.createNestedObject();
  data_4["label"] = "Eindhoven";
  data_4["state"] = true;
  JsonObject data_5 = data.createNestedObject();
  data_5["label"] = "Fresno";
  data_5["state"] = false;
  JsonObject data_6 = data.createNestedObject();
  data_6["label"] = "Genoa";
  data_6["state"] = false;
  JsonObject data_7 = data.createNestedObject();
  data_7["label"] = "Huddersfield";
  data_7["state"] = true;
  JsonObject data_8 = data.createNestedObject();
  data_8["label"] = "Istanbul";
  data_8["state"] = true;
  JsonObject data_9 = data.createNestedObject();
  data_9["label"] = "Jakarta";
  data_9["state"] = false;
  

  //   DEBUG - serialize the document in the serial monitor
  //   serializeJson(doc, Serial);
  //   Serial.println(" ");
    
  char payload[1024]; // length of the char buffer that contains the JSON file, concretely the number of characters included in one message
  size_t payloadSize = serializeJson(doc, payload);
  
  //   DEBUG - write the size of the serialized document
  //   Serial.print("json size:");
  //   Serial.println(payloadSize);
    
  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage("devices/" + device_id + "/messages/events/", static_cast<unsigned long>(payloadSize));
  mqttClient.print(payload);
  mqttClient.endMessage();
  /*  
  To replicate the issue, uncomment the following 3 lines and comment the 3 above. This way you'll only be able to send MQTT messages smaller than 256 Bytes.
  */
  //  mqttClient.beginMessage("devices/" + device_id + "/messages/events/");
  //  serializeJson(doc, mqttClient);
  //  mqttClient.endMessage();
}

void loop() {
  // ================ LOOP FUNCTION ================
  // Select the MQTT topic to subscribe to. It is a default value for DPS.
  // String sub_topic = "$dps/registrations/res/#";
  String sub_topic = "devices/" + device_id + "/messages/devicebound/#";

  // Connect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Establish MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT(sub_topic);
  }

  // poll for new MQTT messages and send keep alives
  // mqttClient.poll();

  // publishMessage();

  // delay(10000); //  publish a message roughly every 10 seconds
}

unsigned long getTime() {
  // Get the current time from the WiFi module
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT(String topic) {
  Serial.print("Attempting to connect to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  long int start_time = millis();
  while (!mqttClient.connect(broker, 8883)) {
    delay(5000);
    // Failed, retry
    Serial.print("connectError: ");
    Serial.println(mqttClient.connectError());
    start_time = millis();
  }
  long int end_time = millis();
  Serial.print("Execution time for the connection: "); Serial.print(end_time-start_time); Serial.println(" milliseconds");

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // Subscribe to the given topic
  mqttClient.subscribe(topic);
}

void onMessageReceived(int messageSize) {
  // Message received, print the topic and the message
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("'");

  // Use the stream interface to print the message contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();
}
