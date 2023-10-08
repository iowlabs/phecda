/*  1) Descomentar Línea A y B
 *  2) Configurar Línea B: Y, M, D, h, m, s
 *  3) Programar
 *  4) Comentar Línea A y B
 *  5) Programar
 */

#include <SPI.h>
#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 rtc;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__))); // Línea A
  rtc.adjust(DateTime(2023, 1, 19, 18, 0, 0));  // Línea B
}

void loop () {
 DateTime now = rtc.now();

 Serial.print(now.day());
 Serial.print('/');
 Serial.print(now.month());
 Serial.print('/');
 Serial.print(now.year());
 Serial.print(" ");
 Serial.print(now.hour());
 Serial.print(':');
 Serial.print(now.minute());
 Serial.print(':');
 Serial.print(now.second());
 Serial.println();
 delay(3000);
}
