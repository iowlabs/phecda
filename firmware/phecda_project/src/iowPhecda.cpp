
/*
# SPDX-FileCopyrightText: 2023 iowlabs <contacto@iowlabs.com>
#
# SPDX-License-Identifier: GPL-3.0-or-later.txt
*/

#include "Arduino.h"
#include "iowPhecda.h"


iowPhecda::iowPhecda()
{

}

uint8_t iowPhecda::begin()
{
  pinMode(LED,OUTPUT);
  pinMode(PH_EN,OUTPUT);
  pinMode(ORP_EN,OUTPUT);
  pinMode(OD_EN,OUTPUT);
  pinMode(EC_EN,OUTPUT);

  pinMode(SD_CS, OUTPUT);

  digitalWrite(PH_EN  ,LOW);
  digitalWrite(ORP_EN ,LOW);
  digitalWrite(OD_EN ,LOW);
  digitalWrite(EC_EN ,LOW);
  digitalWrite(SD_CS  ,HIGH);
  Wire.begin();

  delay(200);

  EEPROM.begin(sizeof(intentosSD));
  EEPROM.get(0, intentosSD);
  Serial.print("Intentos SD:");
  Serial.println(intentosSD);
  if(intentosSD > 1){
    EEPROM.put(0, 0);
    EEPROM.commit();
  }
  if(rtc.begin())      rtc_status = true;
//  if (intentosSD <= 1) {
//    if(iowsd.uSD_init(SD_CS)){
//      sd_status = true;
//    }
//    else{
//      Serial.println("Error al inicializar la tarjeta SD");
//      intentosSD++;
//     EEPROM.put(0, intentosSD);
//      EEPROM.commit();
//      delay(1200);
//      ESP.restart();
//    }
//  }
  if(display.begin(DISPLAY_ADDRESS,true)) display_status = true ;
  if(lora_sel)
  {
    LoRa.setPins(RFM_CS, RFM_RST, RFM_DIO0);
    if(LoRa.begin(915E6)) lora_status = true;
    LoRa.setSyncWord(0xF3);
  }
  display.setRotation(0); //IF NOT INVERTED COMMENT THIS LINE
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);



  if( rtc_status && display_status && sd_status) return STATUS_OK;
  else return STATUS_ERROR;
}



void iowPhecda::iowLogo()
{
  display.clearDisplay(); // Clear the buffer
  display.drawBitmap(0, 0, logo_iowlabs, 128, 64,1,0);
  display.display();
}

void iowPhecda::showLogo()
{
  display.clearDisplay(); // Clear the buffer
  display.drawBitmap(0, 0, logo_phecda, 128, 64,1,0);
  display.display();
}

void iowPhecda::showStatus()
{
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("SD:");
  display.setCursor(80, 0);
  if(sd_status) display.print("OK");
  else display.print("error");
  display.display();
  delay(200);
  display.setCursor(0, 16);
  display.println("RTC:");
  display.setCursor(80, 16);
  if(rtc_status) display.print("OK");
  else display.print("error");
  display.display();
  delay(200);
  display.setCursor(0, 32);
  display.println("SENSORS:");
  if(ph_sel){display.setCursor(0, 48); display.print("PH");display.display();}
  if(orp_sel){display.setCursor(26, 48); display.print("ORP");}
  if(temp_sel){display.setCursor(62, 48); display.print("T");}
  if(ec_sel){display.setCursor(80, 48); display.print("EC");}
  if(od_sel){display.setCursor(110, 48); display.print("OD");}
  display.display();
  delay(200);
}

void iowPhecda::showData(long time_interval)
{
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("PH:");
  display.setCursor(50, 0);
  display.print(ph);
  display.setCursor(0, 16);
  display.println("ORP:");
  display.setCursor(50, 16);
  display.print(int(orp));
  display.setCursor(0, 32);
  display.println("T:");
  display.setCursor(50, 32);
  display.print(temp);
  display.display();
  delay(time_interval);
  display.clearDisplay();
  if (ec_sel)
  {
	  display.setCursor(0, 0);
	  display.println("EC:");
	  display.setCursor(50, 0);
	  display.print(int(ec));

  }
  if(od_sel)
  {
	  display.setCursor(0, 16);
	  display.println("OD:");
	  display.setCursor(50, 16);
	  display.print(int(od));
  }
  if( ec_sel or od_sel)
  {
	  display.display();
	  delay(time_interval);
  	  display.clearDisplay();
	  display.display();
  }


}

void iowPhecda::activatePH(){ph_sel = true;}
void iowPhecda::activateORP(){orp_sel = true;}
void iowPhecda::activateTEMP(){temp_sel = true;}
void iowPhecda::activateEC(){ec_sel = true;}
void iowPhecda::activateOD(){od_sel = true;}
void iowPhecda::activateLoRa(){lora_sel = true;}

void iowPhecda::activateAll()
{
  ph_sel    = true;
  orp_sel   = true;
  temp_sel  = true;
  od_sel    = true;
  ec_sel    = true;
}

void iowPhecda::readSensors()
{

  readAtlasSensors(ph_sel,orp_sel,temp_sel,od_sel,ec_sel);
  if (rtc.updateTime() == true) //Updates the time variables from RTC
  {
    timestamp = rtc.getEpoch(); //Get the time in UNIX
  }
}

void iowPhecda::readAtlasSensors( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s)
{
  digitalWrite(PH_EN,ph_s);
  digitalWrite(ORP_EN,orp_s);
  digitalWrite(OD_EN,od_s);
  digitalWrite(EC_EN,ec_s);

  delay(200);

  if(temp_s)
  {
    temp_sensor.send_read_cmd();
    delay(1000);
    temp_sensor.receive_read_cmd();
    if ((temp_sensor.get_error() == Ezo_board::SUCCESS )&& (temp_sensor.get_last_received_reading() > -1000.0)&& temp_s)
    {
      temp =  temp_sensor.get_last_received_reading();
      temp = (int)(temp*100+0.5)/100.00;
      if(ph_s)ph_sensor.send_cmd_with_num("T,",temp);
      if(orp_s)orp_sensor.send_cmd_with_num("T,",temp);
      if(ec_s)ec_sensor.send_cmd_with_num("T,",temp);
      if(od_s)do_sensor.send_cmd_with_num("T,",temp);
    }
    else
    {
      temp = -90.0;
      if(ph_s)ph_sensor.send_cmd_with_num("T,",25.0);
      if(orp_s)orp_sensor.send_cmd_with_num("T,",25.0);
      if(ec_s)ec_sensor.send_cmd_with_num("T,",25.0);
      if(od_s)do_sensor.send_cmd_with_num("T,",25.0);
    }
  }
  else
  {
    if(ph_s)ph_sensor.send_cmd_with_num("T,",25.0);
    if(orp_s)orp_sensor.send_cmd_with_num("T,",25.0);
    if(ec_s)ec_sensor.send_cmd_with_num("T,",25.0);
    if(od_s)do_sensor.send_cmd_with_num("T,",25.0);
  }
  delay(300);
  if(ph_s)ph_sensor.send_read_cmd();
  if(orp_s)orp_sensor.send_read_cmd();
  if(ec_s)ec_sensor.send_read_cmd();
  if(od_s)do_sensor.send_read_cmd();
  delay(1000);
  if(ph_s)
  {
    ph_sensor.receive_read_cmd();
    if (ph_sensor.get_error() == Ezo_board::SUCCESS)
    {
      ph =  ph_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
      ph = (int)(ph*100+0.5)/100.0;
    }
    else{ ph = -25.0;}
  }
  if(orp_s)
  {
    orp_sensor.receive_read_cmd();
    if (orp_sensor.get_error() == Ezo_board::SUCCESS)
    {
      orp =  orp_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
      orp = (int)(orp*100+0.5)/100.0;
    }
    else{ orp = -25.0;}
  }
  if(od_s)
  {
    do_sensor.receive_read_cmd();
    if (do_sensor.get_error() == Ezo_board::SUCCESS)
    {
      od =  do_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
      od = (int)(od*100+0.5)/100.0;
    }
    else{ od = -25.0;}
  }
  if(ec_s)
  {
    ec_sensor.receive_read_cmd();
    if (ec_sensor.get_error() == Ezo_board::SUCCESS)
    {
      ec =  ec_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
      ec = (int)(ec*100+0.5)/100.0;
    }
    else{ ec = -25.0;}
  }

  digitalWrite(PH_EN,LOW);
  digitalWrite(ORP_EN,LOW);
  digitalWrite(OD_EN,LOW);
  digitalWrite(EC_EN,LOW);
}

String iowPhecda::pubData(void)
{
  StaticJsonDocument<128> doc_tx;

  doc_tx["id"]    = ID;
  doc_tx["time"]  = timestamp;
  if(temp_sel)doc_tx["temp"]  = round2(temp);
  if(ph_sel)  doc_tx["ph"]    = round2(ph);
  if(orp_sel) doc_tx["orp"]   = round2(orp);
  if(od_sel)  doc_tx["od"]    = round2(od);
  if(ec_sel)  doc_tx["ec"]    = round2(ec);

  

  String json;
  serializeJson(doc_tx, json);

  if(lora_sel)
  {
    delayMicroseconds(random(1000)); // random delay for avoid collisions
    LoRa.beginPacket();
    LoRa.print(json);
    LoRa.endPacket();
  }
  return json;
}

void iowPhecda::saveData(void)
{
  StaticJsonDocument<128> doc_tx;

  doc_tx["id"]    = ID;
  doc_tx["time"]  = timestamp;
  if(temp_sel)doc_tx["temp"]  = temp;
  if(ph_sel)  doc_tx["ph"]    = ph;
  if(orp_sel) doc_tx["orp"]   = orp;
  if(od_sel)  doc_tx["od"]    = od;
  if(ec_sel)  doc_tx["ec"]    = ec;

  String json;
  serializeJson(doc_tx, json);
  iowsd.appendFile(SD,FILE_NAME,json.c_str());
  iowsd.appendFile(SD,FILE_NAME,"\n");
}

void iowPhecda::phCalClear(void)
{
  ph_sensor.send_cmd("Cal,clear");
}

void iowPhecda::phCal(uint8_t val)
{
  if(val == 7)ph_sensor.send_cmd("Cal,mid,7.00");
  if(val == 4)ph_sensor.send_cmd("Cal,low,4.00");
  if(val == 10)ph_sensor.send_cmd("Cal,high,10.00");

}
void iowPhecda::PMP_blue()
{
  pmp_blue.send_cmd_with_num("d,", 0.5);
}
void iowPhecda::PMP_red()
{
  pmp_red.send_cmd_with_num("d,", -0.5);
}

void iowPhecda::PMP_blue_action(const char* comand, const char* valor, int min)
{
  if(strcmp(comand,"x,")==0){
    printlnd("modo detener");
    pmp_blue.send_cmd("x");
  }
  else if(strcmp(valor,"?")==0){
    size_t longitudTotal = strlen(comand) + strlen(valor) + 1;
    char buffer[longitudTotal];
    snprintf(buffer, sizeof(buffer), "%s%s", comand, valor);
    const char* resultado = buffer;
    printlnd("modo ?");
    printlnd(resultado);
    pmp_blue.send_cmd(resultado);
  }
  else{
    float numero = atof(valor);
    printlnd(comand);
    printlnd(numero);
    printlnd("modo controlado");
    printlnd(min);
    if (min == 0){
      pmp_blue.send_cmd_with_num(comand, numero);
    }
    else{
      char buffer[20];
      snprintf(buffer, sizeof(buffer), "%s,%.3f,%d", comand, numero, min);
      printlnd(buffer);
      const char* mensaje = buffer;
	    pmp_blue.send_cmd(mensaje);
    }
    
  }
}
void iowPhecda::PMP_red_action(const char* comand, const char* valor, int min)
{
  if(strcmp(comand,"x,")==0){
    printlnd("modo detener");
    pmp_red.send_cmd("x");
  }
  else if(strcmp(valor,"?")==0){
    size_t longitudTotal = strlen(comand) + strlen(valor) + 1;
    char buffer[longitudTotal];
    snprintf(buffer, sizeof(buffer), "%s%s", comand, valor);
    const char* resultado = buffer;
    printlnd("modo ?");
    printlnd(resultado);
    pmp_red.send_cmd(resultado);
  }
  else{
    float numero = atof(valor);
    printlnd(numero);
    printlnd("modo controlado");
    if (min == 0){
      pmp_red.send_cmd_with_num(comand, numero);
    }
    else{
      char buffer[20];
      snprintf(buffer, sizeof(buffer), "%s,%.3f,%d", comand, numero, min);
      printlnd(buffer);
      const char* mensaje = buffer;
	    pmp_red.send_cmd(mensaje);
    }
  }
}


float iowPhecda::round2(float value)
{
    return (int)(value * 100 + 0.5) / 100.0;
}