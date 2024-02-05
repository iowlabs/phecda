/*
# SPDX-FileCopyrightText: 2023 iowlabs <contacto@iowlabs.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later.txt
*/

#ifndef __IOWPHECDA__
#define __IOWPHECDA__


#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Ezo_i2c.h>
#include "Ezo_i2c_util.h"
#include <SparkFun_RV8803.h>
#include "uSD_iow.h"
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <LoRa.h>
#include <WiFi.h>
//#include <PubSubClient.h>



/*-------PINs OF PHECDA BOARD------*/
#define LED          2 // on board led
#define I2C_SCL      21
#define I2C_SDA      22
#define SD_CS     5
#define PH_EN        33
#define ORP_EN       32
#define OD_EN        12
#define EC_EN        13
#define RFM_CS       15
#define RFM_RST      4
#define RFM_DIO0     17
#define RFM_DIO1     14

#define PH_ADDRESS    100
#define ORP_ADDRESS   101
#define TEMP_ADDRESS  102
#define OD_ADDRESS    103
#define EC_ADDRESS    104
#define PMP_BLUE_ADDRESS   105
#define PMP_RED_ADDRESS    106

#define DISPLAY_ADDRESS 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define ID              "n01"
#define RESPONSE_OK     "OK"
#define RESPONSE_ERROR  "ERROR"
#define FILE_NAME       "/log.txt"

#define STATUS_OK     0
#define STATUS_ERROR  1
#define ERROR_SD      0b00000001
#define ERROR_RTC     0b00000010
#define ERROR_OLED    0b00000100
#define ERROR_LORA    0b00001000


//Macros for enable serial prints

#define DEBUG     1

#if DEBUG
#define printd(s) {Serial.print((s));}
#define printlnd(s) {Serial.println((s));}
#else
#define printd(s)
#define printlnd(s)
#endif

//MQTT stufs

#define MQTT_PORT         1883
#define MQTT_USER         "iowlabs"
#define MQTT_PASSWORD     "!iow_woi!"
#define MQTT_PUBLISH_CH   "phecda/msg"
#define MQTT_RECEIVER_CH  "phecda/cmd"


// WiFi and MQTT parameters
#define MQTT_PORT        1883
#define wifi_ssid        "iownwater"
#define wifi_password    "temp3_NL156$"
#define MQTT_SERVER      "35.223.234.244"

// Logo IoWLabs



const unsigned char logo_iowlabs [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x03, 0xf0, 0x0f, 0xc0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x0f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xe3, 0xfc, 0x07, 0xe3, 0xf0, 0xfc, 0x7f, 0x1e, 0x00, 0x00, 0xc0, 0x1e, 0x60, 0x01, 0xc0,
	0xff, 0xe1, 0xf0, 0x01, 0xe3, 0xf0, 0xfc, 0x7f, 0x1e, 0x00, 0x07, 0xf8, 0x1f, 0xf8, 0x0f, 0xf8,
	0xff, 0xe1, 0xe0, 0x40, 0xe1, 0xf0, 0x7c, 0x7f, 0x1e, 0x00, 0x0f, 0xfc, 0x1f, 0xfc, 0x1f, 0xfe,
	0xff, 0xe1, 0xe1, 0xf0, 0xf1, 0xe0, 0x78, 0x7f, 0x1e, 0x00, 0x0f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe,
	0xff, 0xe1, 0xc3, 0xf8, 0x71, 0xe0, 0x78, 0xff, 0x1e, 0x00, 0x06, 0x1e, 0x1f, 0x1e, 0x3e, 0x1c,
	0xff, 0xe1, 0xc3, 0xf8, 0x70, 0xe0, 0x38, 0xff, 0x1e, 0x00, 0x00, 0x0e, 0x1e, 0x0e, 0x3c, 0x00,
	0xff, 0xe1, 0xc3, 0xf8, 0x78, 0xc2, 0x30, 0xff, 0x1e, 0x00, 0x00, 0x0e, 0x1e, 0x0e, 0x3f, 0x00,
	0xff, 0xe1, 0xc3, 0xf8, 0x78, 0xc6, 0x31, 0xff, 0x1e, 0x00, 0x07, 0xfe, 0x1e, 0x0e, 0x1f, 0xf8,
	0xff, 0xe1, 0xc3, 0xf8, 0x78, 0x46, 0x31, 0xff, 0x1e, 0x00, 0x0f, 0xfe, 0x1e, 0x0e, 0x1f, 0xfc,
	0xff, 0xe1, 0xc3, 0xf8, 0x7c, 0x47, 0x01, 0xff, 0x1e, 0x00, 0x1f, 0xfe, 0x1e, 0x0e, 0x07, 0xfe,
	0xff, 0xe1, 0xc3, 0xf8, 0x7c, 0x0f, 0x03, 0xff, 0x1e, 0x00, 0x1e, 0x0e, 0x1e, 0x0e, 0x00, 0x1e,
	0xff, 0xe1, 0xe1, 0xf0, 0xfc, 0x0f, 0x03, 0xff, 0x1e, 0x00, 0x1c, 0x0e, 0x1e, 0x0e, 0x10, 0x1e,
	0xff, 0xe1, 0xe0, 0x40, 0xfe, 0x0f, 0x83, 0xff, 0x1e, 0x00, 0x1e, 0x0e, 0x1e, 0x1e, 0x1c, 0x1e,
	0xff, 0xe1, 0xf0, 0x01, 0xfe, 0x0f, 0x87, 0xff, 0x1f, 0xff, 0x9f, 0xfe, 0x1f, 0xfe, 0x3f, 0xfe,
	0xff, 0xe3, 0xfc, 0x07, 0xff, 0x1f, 0x87, 0xff, 0x1f, 0xff, 0x9f, 0xfe, 0x1f, 0xfc, 0x3f, 0xfc,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0x8f, 0xfe, 0x1f, 0xfc, 0x1f, 0xf8,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x0f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x03, 0xf0, 0x0f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 'Phecda_logo', 128x64px
const unsigned char logo_phecda [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xff, 0xff, 0xc7, 0x07, 0x00, 0xe0, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0xe0, 0x00, 0x00,
	0x00, 0x01, 0xff, 0xff, 0xef, 0x87, 0x87, 0xf8, 0x01, 0xf8, 0x3f, 0xf0, 0x01, 0xf0, 0x00, 0x00,
	0x00, 0x01, 0xff, 0x87, 0xef, 0x8f, 0x8f, 0xfc, 0x07, 0xfc, 0x7f, 0xf8, 0x01, 0xf0, 0x00, 0x00,
	0x00, 0x01, 0xef, 0x83, 0xef, 0x8f, 0x9f, 0xfe, 0x0f, 0xfc, 0xff, 0xfc, 0x03, 0xf0, 0x00, 0x00,
	0x00, 0x00, 0xcf, 0x83, 0xff, 0x8f, 0xbf, 0xff, 0x1f, 0xdd, 0xff, 0xfe, 0x07, 0xf0, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0x83, 0xff, 0x8f, 0xbf, 0x0f, 0x3f, 0x0f, 0xff, 0x3e, 0x07, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0x83, 0xff, 0x0f, 0xfe, 0x0f, 0x3e, 0x03, 0xff, 0x1e, 0x0f, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0x03, 0xff, 0x0f, 0xfc, 0x0f, 0x7e, 0x07, 0xde, 0x1f, 0x0f, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0x03, 0xff, 0x0f, 0xfc, 0x0e, 0x7c, 0x07, 0xbe, 0x1f, 0x1f, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0x03, 0xff, 0x0f, 0xfc, 0x04, 0xfc, 0x03, 0x3e, 0x1e, 0x1f, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x3f, 0x03, 0xff, 0x0f, 0xfc, 0x00, 0xf8, 0x00, 0x3e, 0x3e, 0x3f, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x3f, 0x07, 0xff, 0x0f, 0xfc, 0x00, 0xf8, 0x00, 0x3e, 0x3e, 0x3e, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x3e, 0x0f, 0xff, 0xff, 0xfe, 0x00, 0xf0, 0x00, 0x3e, 0x7e, 0x7e, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x3e, 0x0f, 0xdf, 0xff, 0xbf, 0x81, 0xf0, 0x00, 0x3c, 0x7c, 0x7f, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x3e, 0x1f, 0xdf, 0xff, 0xbf, 0x81, 0xf0, 0x0c, 0x7c, 0xfc, 0x7f, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x3e, 0x7f, 0x9f, 0xff, 0x3f, 0xc1, 0xf0, 0x3c, 0x7d, 0xf8, 0xff, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0xff, 0x1f, 0xff, 0xff, 0x81, 0xf0, 0x3c, 0x7f, 0xf0, 0xff, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0xfe, 0x1e, 0x0f, 0xff, 0x01, 0xf0, 0x7c, 0x7f, 0xf1, 0xf3, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0xfc, 0x1e, 0x1f, 0xfc, 0x01, 0xf0, 0xfc, 0x7f, 0xe1, 0xf0, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0xf8, 0x1e, 0x1f, 0xf8, 0x01, 0xf1, 0xf8, 0x7f, 0xc1, 0xe0, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0xe0, 0x3e, 0x1f, 0xf9, 0xe1, 0xfb, 0xf8, 0x7f, 0x81, 0xe0, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7f, 0x80, 0x3e, 0x1f, 0xff, 0xe1, 0xff, 0xf0, 0x7f, 0x03, 0xc0, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7c, 0x00, 0x3e, 0x1f, 0x7f, 0xe0, 0xff, 0xe0, 0xfe, 0x03, 0xc0, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7c, 0x00, 0x3e, 0x1f, 0x7f, 0xc0, 0x7f, 0xc0, 0xfc, 0x03, 0xc0, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7c, 0x00, 0x3e, 0x0f, 0x3f, 0x00, 0x3f, 0x80, 0x70, 0x00, 0x00, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x7c, 0x00, 0x08, 0x04, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
	0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


class iowPhecda
{
  public:
    iowPhecda();


    float ph    = 0.0;
    float temp  = 0.0;
    float orp   = 0.0;
    float ec    = 0.0;
    float od    = 0.0;

    unsigned long timestamp;

    void readAtlasSensors( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s);
    uint8_t begin(void);
    void readSensors(void);
    String pubData(void);
    void activatePH(void);
    void activateORP(void);
    void activateTEMP(void);
    void activateEC(void);
    void activateOD(void);
    void activateLoRa(void);
	void activatePMP_blue(void);
	void activatePMP_red(void);
    void activateAll(void);
    void iowLogo(void);
    void showLogo(void);
    void showStatus(void);
    void showData(long time_interval);
    void saveData(void);
    void phCal(uint8_t val);
    void phCalClear(void);
	void PMP_blue(void);
	void PMP_red(void);


  private:
    RV8803 rtc;
    USD_IOW iowsd;
    Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    Ezo_board ph_sensor   = Ezo_board(PH_ADDRESS,"PH");
    Ezo_board orp_sensor  = Ezo_board(ORP_ADDRESS,"ORP");
    Ezo_board temp_sensor = Ezo_board(TEMP_ADDRESS,"RTD");
    Ezo_board do_sensor   = Ezo_board(OD_ADDRESS,"DO");
    Ezo_board ec_sensor   = Ezo_board(EC_ADDRESS,"EC");
	Ezo_board pmp_blue 		  = Ezo_board(PMP_BLUE_ADDRESS,"PMP_BLUE");
	Ezo_board pmp_red 		  = Ezo_board(PMP_RED_ADDRESS,"PMP_RED");
    bool ph_sel = false;
    bool orp_sel = false;
    bool temp_sel = false;
    bool od_sel = false ;
    bool ec_sel = false;
    bool lora_sel = false;
    bool rtc_status = false;
    bool sd_status = false;
    bool display_status = false;
    bool lora_status = false;
	bool pmp_blue_sel = false;
	bool pmp_red_sel = false;


};

#endif
