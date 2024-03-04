/*
# SPDX-FileCopyrightText: 2023 iowlabs <contacto@iowlabs.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later.txt
*/

#include <Arduino.h>
#include "iowPhecda.h"

iowPhecda phecda = iowPhecda();
uint8_t status;
String output;

void setup()
{
  Serial.begin(115200);
  phecda.activatePH();
  phecda.activateTEMP();
  phecda.activateORP();
  phecda.activateLoRa();

  status = phecda.begin();
  printlnd(status);

  delay(200);
  phecda.iowLogo();
  delay(2000);
  phecda.showLogo();
  delay(2000);

}

void loop()
{
  phecda.readSensors();
  output = phecda.pubData();// when lora is active, this funtion send the string output by LoRa
  Serial.println(output);
  phecda.saveData();
  phecda.showData(2500);
}
