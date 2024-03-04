/*
# SPDX-FileCopyrightText: 2023 iowlabs <contacto@iowlabs.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later.txt
*/

#include <Arduino.h>
#include "iowPhecda.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>


//Wifi and mqtt client
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

iowPhecda phecda = iowPhecda();
uint8_t status;
String output;
int  mqtt_try       = 0; // Cantidad de intentos para conectarse
int conteo_sec = 0;
bool mqtt_connected = false;
bool wifi_status = false;

const char* motor;
const char* cmd;
const char* response = RESPONSE_OK;

void setupWiFi()
{
  delay(10);
  printd("Connecting to ");
  printlnd(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    printd(".");

  }
  printlnd("WiFi connected");
  printd("IP address: ");
  printlnd(WiFi.localIP());
  digitalWrite(LED,HIGH);
}

// Funcion para reconectar a la red en caso que se pierda la conexion
bool reconnect()
{
    while (!mqtt.connected())
    {
        printd("Attempting MQTT connection...");
        if (mqtt.connect(ID, MQTT_USER , MQTT_PASSWORD))
        {
            printlnd("connected");
            mqtt.subscribe(MQTT_RECEIVER_CH);
            mqtt_try = 0;
            return true;
        }
        else
        {
            printd("failed, rc=");
            printlnd(mqtt.state());
            printlnd(" try again in 5 seconds");
            delay(5000);
            mqtt_try += 1;
            if(mqtt_try>=3)
            {
              mqtt_try = 0;
              return false;
            }
        }
    }

    return false;
}


void publishMqtt(char *payload)
{
  if(!mqtt.connected())
  {
    reconnect();
  }
  mqtt.publish(MQTT_PUBLISH_CH, payload);

}

void processCmd(byte* payload, unsigned int length)
{
	StaticJsonDocument<256> doc_rx;
    //const char* json_rx = "{\"id\":\"anw00\",\"cmd\":\"gps\",\"arg\":1}";
    DeserializationError error_rx;
    //check for error
    error_rx = deserializeJson(doc_rx, payload,length);
    if (error_rx)
    {
		printd(F("deserializeJson() failed: "));
		response = RESPONSE_ERROR_JSON;
      	printlnd(error_rx.c_str());

    }

    //parsing incoming msg

    motor = doc_rx["motor"];
    if( strcmp(motor,"B")==0)
    {
      cmd = doc_rx["cmd"];
      const char* val = doc_rx["val"];
      int min = doc_rx["min"];
      phecda.PMP_blue_action(cmd,val, min);
    }
    else if( strcmp(motor,"R")==0)
    {
      cmd = doc_rx["cmd"];
      const char* val = doc_rx["val"];
      int min = doc_rx["min"];
      phecda.PMP_red_action(cmd,val, min);
    }
    else
    {
      printlnd("msg not for me");
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
	if(DEBUG)
  	{
    	Serial.println("-------new message from broker-----");
    	Serial.print("channel:");
    	Serial.println(topic);
    	Serial.print("data:");
    	Serial.write(payload, length);
    	Serial.println();
  	}
    processCmd(payload,length);
}

void setup()
{
  Serial.begin(115200);
  phecda.activatePH();
  phecda.activateTEMP();
  phecda.activatePMP_blue();
  phecda.activatePMP_red();
  //phecda.activateORP();

  status = phecda.begin();

  setupWiFi();
  mqtt.setServer(MQTT_SERVER,MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  printlnd(status);
  /*
  delay(200);
  phecda.iowLogo();
  delay(1000);
  phecda.showLogo();
  delay(1000);
  phecda.showStatus();
  delay(1000);
  */
}

void loop()
{
  phecda.readSensors();
  output = phecda.pubData();
  Serial.println(output);
  //phecda.saveData();
  //phecda.showData(2500);
  publishMqtt((char*) output.c_str());
  mqtt.loop();
}



