/*
# SPDX-FileCopyrightText: 2023 iowlabs <contacto@iowlabs.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later.txt
*/

#include <Arduino.h>
#include "iowPhecda.h"

#define SLEEP_TIME  60   // in seconds
#define TIME_FACTOR 1000000

iowPhecda phecda = iowPhecda();
uint8_t status;
String output;

void    mainTask(void);

void setup()
{
  Serial.begin(115200);
  phecda.activatePH();
  phecda.activateTEMP();
  phecda.activateORP();

  status = phecda.begin();


  printlnd(status);
  delay(200);
  phecda.iowLogo();
  delay(1000);
  phecda.showLogo();
  delay(1000);
  phecda.showStatus();
  delay(2500);

  mainTask();
}

void loop(){}


void mainTask()
{
  phecda.readSensors();
  output = phecda.pubData();
  Serial.println(output);
  phecda.saveData();
  phecda.showData(2500);

  esp_sleep_enable_timer_wakeup(SLEEP_TIME*TIME_FACTOR);
  esp_deep_sleep_start();
}
