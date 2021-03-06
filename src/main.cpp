/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <FS.H>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "settings.h"


//Global objects
WiFiClientSecure espClient;
PubSubClient client(espClient);
long lastMsg = 0;

//Function declarations
void setup_sensor();
void setup_wifi();
void setup_mqtt();
void reconnect();



void setup_sensor() {
  // 
}

void setup_wifi() {
  Serial.print("Connecting to "); 
  Serial.print(ssid);
  
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  
  Serial.print(", WiFi connected, IP address: "); 
  Serial.println(WiFi.localIP());
}

void setup_mqtt() {
  //Initalize TLS certficate and private key
  if(SPIFFS.begin() == false) {
    Serial.println(F("SPIFFS.begin() failed"));
    while(true);
  }

  File certFile = SPIFFS.open(certificate_path, "r");
  if (!certFile) {
    Serial.println(F("Couldn't load cert"));
    while(true) delay(10);
  }
  espClient.loadCertificate(certFile,certFile.size());

  File privateKeyFile = SPIFFS.open(private_key_path, "r");
  espClient.loadPrivateKey(privateKeyFile,privateKeyFile.size());
  if (!privateKeyFile) {
    Serial.println(F("Couldn't load key"));
    while(true) delay(10);
  }

  SPIFFS.end();


  //Set destination MQTT server
  client.setServer(mqtt_server, mqtt_port);
}

void setup() {
  delay(300);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200); 

  setup_wifi();
  setup_mqtt();
  setup_sensor();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client_id)) {
      
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {

      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);

    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2500) {

    lastMsg = now;

    // Create JSON Object
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["valueFloat"] = 1.5f;
    root["valueInt"] = 1;
    root["valueString"] = "IotCamp";

    //Write JSON object to string
    int outputBufferLen = root.measureLength() + 1;
    char outputBuffer[outputBufferLen];
    root.printTo(outputBuffer,outputBufferLen);
    
    //Send MQTT Message
    Serial.print("Publish message: ");
    Serial.println(outputBuffer);
    client.publish(mqtt_publish_topic, outputBuffer);
  }

}

