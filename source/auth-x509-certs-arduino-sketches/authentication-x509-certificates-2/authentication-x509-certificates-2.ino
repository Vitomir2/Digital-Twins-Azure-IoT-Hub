/*
   Azure IoT Device in the Azure IoT Hub
   This sketch securely connects to an device in the Azure IoT Hub using MQTT
   over WiFi, secured by SSl.
   It uses a private key stored in the built-in crypto chip and a CA signed
   public certificate for SSL/TLS authetication.
   It subscribes to a IoT Hub topic to receive the response, and publishes a message
   to stard the device enrollment.
   Additionally, there is a timer for the execution time of the successful authenticaiton
   and it is printed on the serial console.
   Boards:
   - Arduino MKR 1010 WiFi
   Author: Vitomir Pavlov
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

// ================================== Settings ===================================
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
const String id_scope  = SECRET_ID_SCOPE;
const String device_id = SECRET_DEVICE_ID;

WiFiClient    wifi_client;            // Used for the TCP socket connection
BearSSLClient ssl_client(wifi_client); // Used for SSL/TLS connection, integrates with ECCX08
MqttClient    mqtt_client(ssl_client); // Used for MQTT protocol usage

// ================================== Auxiliary functions ===================================
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
  while (!mqtt_client.connect(broker, 8883)) {
    delay(5000);
    // Failed, retry
    Serial.print("connectError: ");
    Serial.println(mqtt_client.connectError());
    start_time = millis();
  }
  long int end_time = millis();
  Serial.print("Execution time for the connection: "); Serial.print(end_time - start_time); Serial.println(" milliseconds");

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // Subscribe to the given topic
  mqtt_client.subscribe(topic);
}

void onMessageReceived(int message_size) {
  // Message received, print the topic and the message
  Serial.print("Received a message with topic '");
  Serial.print(mqtt_client.messageTopic());
  Serial.print("'");

  // Use the stream interface to print the message contents
  while (mqtt_client.available()) {
    Serial.print((char)mqtt_client.read());
  }
  Serial.println();
}

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
    ssl_client.setEccSlot(
      keySlot,
      ECCX08SelfSignedCert.bytes(),
      ECCX08SelfSignedCert.length()
    );
  } else if (!self_signed_cert) {
    Serial.println("Using the certificate from secrets.h...");

    // Instruct the SSL client to use the chosen ECCX08 slot for picking the private key
    // and set the hardcoded certificate as accompanying public certificate.
    // ssl_client.setEccSlot(
    //  keySlot,
    //  CLIENT_PUBLIC_CERT);

    // br_x509_certificate x509cert = {
    //  CLIENT_CERT,
    //  sizeof(CLIENT_CERT) - 1
    // };

    // Set the X509 certificate
    // ssl_client.setEccCert(x509cert);

    // ssl_client.setKey(CLIENT_KEY, CLIENT_CERT);
  }

  /*
     Note: I prefer to use BearSSLClient::setEccSlot because it contains a function to decode
     the .pem certificate (the hardcoded certificate can be stored in base64, instead of
     converting it to binary), and it automatically computes the certificate length.
  */

  // ================ MQTT Client SETUP ================
  // Set the client id used for MQTT as the device_id
  mqtt_client.setId(device_id);

  // Set the username to "<id_scope>/registrations/<registrationId>/api-version=2019-03-31"
  // String username = broker + "/registrations/" + device_id + "/api-version=2019-03-31";
  String username;
  username += broker;
  username += "/";
  username += device_id;
  username += "/?api-version=2018-06-30";

  // Set an empty password if you want to use X.509 authentication, otherwise set the SAS token connection string
  String password = SECRET_CONN_STRING;

  // Authenticate the MQTT Client
  mqtt_client.setUsernamePassword(username, password);

  // Set the on message callback, called when the MQTT Client receives a message
  mqtt_client.onMessage(onMessageReceived);
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
  size_t payload_size = serializeJson(doc, payload);
  
  //   DEBUG - write the size of the serialized document
  //   Serial.print("json size:");
  //   Serial.println(payload_size);
    
  // send message, the Print interface can be used to set the message contents
  mqtt_client.beginMessage("devices/" + device_id + "/messages/events/", static_cast<unsigned long>(payload_size));
  mqtt_client.print(payload);
  mqtt_client.endMessage();
  
  /*
  To replicate the issue, uncomment the following 3 lines and comment the 3 above. This way you'll only be able to send MQTT messages smaller than 256 Bytes.
  */
  //  mqtt_client.beginMessage("devices/" + device_id + "/messages/events/");
  //  serializeJson(doc, mqtt_client);
  //  mqtt_clientnt.endMessage();
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
  if (!mqtt_client.connected()) {
    connectMQTT(sub_topic);
  }

  // poll for new MQTT messages and send keep alives
  // mqtt_client.poll();

  // publishMessage();

  // delay(10000); //  publish a message roughly every 10 seconds
}
