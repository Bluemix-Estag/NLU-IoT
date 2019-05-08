

/**
  IBM IoT Foundation managed Device
  
  Author: Ant Elder
  License: Apache License v2
  
  Modifications copyright 2018 Eduardo Petecof Mattoso

  - Changes:
    - Changed credentials and added new topics
    - Added function handleLEDCommands() to aggregate all my commands and save memory
    - Added function parseJson() to modularize JSON Parsing from original handleUpdates()
  
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
#include <Ethernet.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7

//-------- Customise these values -----------

// ALTERAR OS ## pelo numero do seu kit
byte mac[] = { 0x00, 0xFF, 0xBB, 0xCC, 0xDE, 0x## };
byte ip[] = { 10, 3, 10, 1##};

byte dns[] = { 192, 168, 70, 150};
byte gateway[] = { 10, 3, 10, 254};
byte subnet[] = { 255, 255, 255, 0};
 

#define DIG_OUT 7

#define ORG "vrreja"
#define DEVICE_TYPE "Arduino"
#define DEVICE_ID "Arduino-Risch"
#define TOKEN "4J-cXBwQp2LWgmwXO3"

//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char publishTopic[] = "iot-2/evt/sensor/fmt/json";
const char blinkTopic[] = "iot-2/cmd/blink/fmt/json";
const char turnTopic[] = "iot-2/cmd/turn/fmt/json";

void callback(char* topic, byte* payload, unsigned int payloadLength);

EthernetClient etherClient;
PubSubClient client(server, 1883, callback, etherClient);

int publishInterval = 30000; // 30 seconds
long lastPublishMillis;

void setup() {
  Serial.begin(9600); Serial.println();
  pinMode(DIG_OUT, OUTPUT);
  digitalWrite(DIG_OUT, LOW);
  ethernetConnect();
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

void ethernetConnect() {
 Serial.print("Connecting to Ethernet");
 Ethernet.begin(mac, ip, dns, gateway, subnet); 
 Serial.print("\nConnected, IP address: "); Serial.println(Ethernet.localIP());
 delay(1500);
}

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

void initManagedDevice() {
 if (client.subscribe("iot-2/cmd/+/fmt/json")) {
   Serial.println("subscribe to all commands OK");
 } else {
   Serial.println("subscribe to all commands FAILED");
 }
}

void publishData() {
  
 int sensorValue = analogRead(A0);
 
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

  if (strcmp (blinkTopic, topic) == 0) {
    Serial.println("Doing blink command...");
    handleLEDCommands(payload);
  }
   
  if (strcmp (turnTopic, topic) == 0) {
    Serial.println("Doing turn command...");
    handleLEDCommands(payload);
  } 
}

void handleLEDCommands(byte* payload) {
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

   if (strcmp (fieldName, "blink") == 0) {
     const int fieldValue = field["value"];
     long init = millis();
     while((millis() - init) < (fieldValue * 1000)){
      digitalWrite(DIG_OUT, HIGH); 
      delay(500);                      
      digitalWrite(DIG_OUT, LOW);
      delay(500);
     }
   }

   if (strcmp (fieldName, "turn") == 0) {
     const char* fieldValue = field["value"];
     if(strcmp (fieldValue, "ON") == 0){
        digitalWrite(DIG_OUT, HIGH);
     }
     else{
        digitalWrite(DIG_OUT, LOW);
     }
   }
 }
}
