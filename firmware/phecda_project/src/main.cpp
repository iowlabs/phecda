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

void PMP_fun_wrapper(void* arg) {
  Serial.println("Ejecutando PMP...");
  phecda.PMP_blue();
  phecda.PMP_red();
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

 esp_timer_create_args_t timer_args = {
    .callback = &PMP_fun_wrapper,
    .arg = NULL,
    .name = "my_timer"
  };

  esp_timer_handle_t my_timer;
  esp_timer_create(&timer_args, &my_timer);

  // Configura el temporizador para que se ejecute cada 10 segundos
  esp_timer_start_periodic(my_timer, 10000000); // El tiempo está en microsegundos
}

void loop()
{
  phecda.readSensors();
  output = phecda.pubData();
  Serial.println(output);
  //phecda.saveData();
  //phecda.showData(2500);
  publishMqtt((char*) output.c_str());
}



