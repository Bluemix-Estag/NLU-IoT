/**
  IBM IoT Foundation managed Device
  
  Author: Ant Elder
  License: Apache License v2
  
  Modifications copyright 2018 Eduardo Petecof Mattoso

    - Origin: https://developer.ibm.com/recipes/tutorials/run-an-esp8266arduino-as-a-iot-foundation-managed-device/
  - Changes:
    - Changed credentials and added new topics
    - Added function handleLEDCommands() to aggregate all my commands and save memory
    - Added lines to read Analog Input from LDR
  
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
  
    http://www.apache.org/licenses/LICENSE-2.0
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7

//-------- Customise these values -----------
const char* ssid = "<yourWiFiSSID>";
const char* password = "<yourWiFiPassword>";

#define ORG "<yourOrg>"
#define DEVICE_TYPE "yourDeviceType"
#define DEVICE_ID "yourDevice"
#define TOKEN "yourDeviceToken"
//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

//-------- Topics to listen to -----------
const char publishTopic[] = "iot-2/evt/sensor/fmt/json";
const char responseTopic[] = "iotdm-1/response";
const char manageTopic[] = "iotdevice-1/mgmt/manage";
const char updateTopic[] = "iotdm-1/device/update";
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";
const char blinkTopic[] = "iot-2/cmd/blink/fmt/json";
const char turnTopic[] = "iot-2/cmd/turn/fmt/json";

void callback(char* topic, byte* payload, unsigned int payloadLength);

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 30000; // 30 seconds
long lastPublishMillis;

void setup() {
  Serial.begin(115200); Serial.println();
  pinMode(LED_BUILTIN, OUTPUT); //Setting up to use Built-in LED
  digitalWrite(LED_BUILTIN, HIGH);
  wifiConnect();
  mqttConnect();
  initManagedDevice();
}

void loop() {
 if (millis() - lastPublishMillis > publishInterval) {
   publishData();
   lastPublishMillis = millis();
 }

 if (!client.loop()) {
   mqttConnect();
   initManagedDevice();
 }
}

//-------- Connecting Wifi -----------
void wifiConnect() {
 Serial.print("Connecting to "); Serial.print(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }
 Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

//-------- Connecting MQTT Client -----------
void mqttConnect() {
 if (!!!client.connected()) {
   Serial.print("Reconnecting MQTT client to "); Serial.println(server);
   while (!!!client.connect(clientId, authMethod, token)) {
     Serial.print(".");
     delay(1500);
   }
   Serial.println();
 }
}

//-------- Subscribing to the chosen topics -----------
void initManagedDevice() {
 if (client.subscribe("iotdm-1/response")) {
   Serial.println("subscribe to responses OK");
 } else {
   Serial.println("subscribe to responses FAILED");
 }

 if (client.subscribe(rebootTopic)) {
   Serial.println("subscribe to reboot OK");
 } else {
   Serial.println("subscribe to reboot FAILED");
 }

 if (client.subscribe("iotdm-1/device/update")) {
   Serial.println("subscribe to update OK");
 } else {
   Serial.println("subscribe to update FAILED");
 }

 if (client.subscribe("iot-2/cmd/+/fmt/json")) {
   Serial.println("subscribe to all commands OK");
 } else {
   Serial.println("subscribe to all commands FAILED");
 }

//-------- Publishing Initial Metadata -----------
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
 JsonObject& d = root.createNestedObject("d");
 JsonObject& metadata = d.createNestedObject("metadata");
 metadata["publishInterval"] = publishInterval;
 JsonObject& supports = d.createNestedObject("supports");
 supports["deviceActions"] = true;

 char buff[300];
 root.printTo(buff, sizeof(buff));
 Serial.println("publishing device metadata:"); Serial.println(buff);
 if (client.publish(manageTopic, buff)) {
   Serial.println("device Publish ok");
 } else {
   Serial.print("device Publish failed:");
 }
}

void publishData() {
  
 int sensorValue = analogRead(A0); //Read LDR Value
 Serial.println(sensorValue);
 
 String payload = "{\"d\":{\"counter\":";
 payload += millis() / 1000;
 payload += ",\"sensor\":";
 payload += sensorValue;
 payload += "}}";

 Serial.print("Sending payload: "); Serial.println(payload);

 if (client.publish(publishTopic, (char*) payload.c_str())) {
   Serial.println("Publish OK");
 } else {
   Serial.println("Publish FAILED");
 }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
 Serial.print("callback invoked for topic: "); Serial.println(topic);

 if (strcmp (responseTopic, topic) == 0) {
  StaticJsonBuffer<300> jsonBuffer;
   JsonObject& root = jsonBuffer.parseObject((char*)payload);
   if (!root.success()) {
     Serial.println("handleUpdate: payload parse FAILED");
     return;
   }
   Serial.println("handleUpdate payload:"); root.prettyPrintTo(Serial); Serial.println();
   return; // just print of response for now
 }

 if (strcmp (rebootTopic, topic) == 0) {
   Serial.println("Rebooting...");
   ESP.restart();
 }

  if (strcmp (updateTopic, topic) == 0) {
    handleUpdate(payload);
  }

  if (strcmp (blinkTopic, topic) == 0) {
    Serial.println("Doing blink command...");
    handleLEDCommands(payload);
  }
 
  if (strcmp (turnTopic, topic) == 0) {
    Serial.println("Doing turn command...");
    handleLEDCommands(payload);
  } 
}

//-------- Identify command value -----------
//I'm using the same function for both commands to save code memory
//Feel free to make it better :)

void handleLEDCommands(byte* payload) {
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.parseObject((char*)payload);
 if (!root.success()) {
   Serial.println("handleLEDCommands: payload parse FAILED");
   return;
 }
 Serial.println("handleLEDCommands payload:"); root.prettyPrintTo(Serial); Serial.println();

 JsonObject& d = root["d"];
 JsonArray& fields = d["fields"];
 for (JsonArray::iterator it = fields.begin(); it != fields.end(); ++it) {
   JsonObject& field = *it;
   const char* fieldName = field["field"];

//-------- Blink LED for x seconds -----------
   if (strcmp (fieldName, "blink") == 0) {
     const int fieldValue = field["value"];
     long init = millis();
     while((millis() - init) < (fieldValue * 1000)){
      digitalWrite(LED_BUILTIN, LOW);   
      delay(500);                      
      digitalWrite(LED_BUILTIN, HIGH);  // 
      delay(500);
     }
   }

//-------- Turn LED ON and OFF -----------
   if (strcmp (fieldName, "turn") == 0) {
     const char* fieldValue = field["value"];
     if(strcmp (fieldValue, "ON") == 0){
        digitalWrite(LED_BUILTIN, LOW);
     }
     else{
        digitalWrite(LED_BUILTIN, HIGH);
     }
   }
 }
}

//-------- Update Metadata to change publish interval -----------
void handleUpdate(byte* payload) {
 StaticJsonBuffer<300> jsonBuffer;
 JsonObject& root = jsonBuffer.parseObject((char*)payload);
 if (!root.success()) {
   Serial.println("handleUpdate: payload parse FAILED");
   return;
 }
 Serial.println("handleUpdate payload:"); root.prettyPrintTo(Serial); Serial.println();

 JsonObject& d = root["d"];
 JsonArray& fields = d["fields"];
 for (JsonArray::iterator it = fields.begin(); it != fields.end(); ++it) {
   JsonObject& field = *it;
   const char* fieldName = field["field"];
   if (strcmp (fieldName, "metadata") == 0) {
     JsonObject& fieldValue = field["value"];
     if (fieldValue.containsKey("publishInterval")) {
       publishInterval = fieldValue["publishInterval"];
       Serial.print("publishInterval:"); Serial.println(publishInterval);
     }
   }
 }
}