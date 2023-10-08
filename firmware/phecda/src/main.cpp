#include <Arduino.h>
#include "iowPhecda.h"


iowPhecda phecda = iowPhecda();
uint8_t status;
String output;

void setup()
{
  Serial.begin(115200);
  status = phecda.begin();
  phecda.activatePH();
  phecda.activateTEMP();
  phecda.activateORP();

  printlnd(status);
  delay(200);
  phecda.iowLogo();
  delay(3000);
  phecda.showLogo();
  delay(3000);
  phecda.showStatus();
  delay(3000);
}

void loop()
{
  phecda.readSensors();
  output = phecda.pubData();
  Serial.println(output);
  phecda.saveData();
  phecda.showData(2500);

}
