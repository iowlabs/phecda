/*
# SPDX-FileCopyrightText: 2023 iowlabs <contacto@iowlabs.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later.txt
*/

#include <Arduino.h>
#include "iowPhecda.h"
#include <PubSubClient.h>

#define TIME_TO_SLEEP 60 //IN SECONDS
#define TIME_FACTOR   1000000


// Parametros de MQTT  USER DEFINE

#define MQTT_PORT         1883
#define MQTT_USER         "iowlabs"
#define MQTT_PASSWORD     "!iow_woi!"
#define MQTT_PUBLISH_CH   "testph"
#define MQTT_RECEIVER_CH  "testph/rx"

//Wifi and mqtt client
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// Conexión a una red y servidor de MQTT
const char* ssid      	= "iownwater";
const char* password  	= "temp3_NL156$";
const char* mqtt_server = "35.223.234.244";

iowPhecda phecda = iowPhecda();
uint8_t status;
String output;
int  mqtt_try       = 0; // Cantidad de intentos para conectarse
bool mqtt_connected = false;
bool wifi_status = false;

void    publishMqtt(char *serialData);
void    setupWiFi();
bool    reconnect();


void setup()
{
  	Serial.begin(115200);
  	phecda.activatePH();
  	phecda.activateTEMP();
  	phecda.activateORP();

  	status = phecda.begin();

  	printlnd(status);

  	delay(1000);
	setupWiFi();
	mqtt.setServer(mqtt_server,MQTT_PORT);
}

void loop()
{
	phecda.readSensors();
  	output = phecda.pubData();
  	Serial.println(output);
  	phecda.saveData();
  	phecda.showData(2500);
  	publishMqtt((char*) output.c_str());
}



void setupWiFi()
{
  delay(10);
  printd("Connecting to ");
  printlnd(ssid);
  WiFi.begin(ssid, password);

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

  WiFi.disconnect(true);
}
