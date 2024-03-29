#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "iowPhecda.h"

// defines para deep sleep
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60//60*60*1        /* Time ESP32 will go to sleep (in seconds) */

bool flag_TXCOMPLETE = false;

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0x84, 0x5C, 0x87, 0x37, 0x23, 0x88, 0xFE, 0xF6 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0xB4, 0x18, 0x01, 0x94, 0x0B, 0x08, 0x8A, 0x82 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x27, 0x88, 0x42, 0x08, 0xA1, 0x8F, 0xA6, 0xEF, 0x7A, 0x42, 0xFE, 0x0E, 0xB7, 0x68, 0x82, 0x09 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[] = "data";
static osjob_t sendjob;

iowPhecda phecda = iowPhecda();
String output;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30; //60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 15,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {17, 14, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 8,              // LBT cal for the Adafruit Feather M0 LoRa, in dB
    .spi_freq = 8000000,
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void do_send(osjob_t* j)
{
	phecda.readSensors();
	output = phecda.pubData();
	Serial.println(output);
	//phecda.saveData();
	Serial.println("DO SEND");
    if (LMIC.opmode & OP_TXRXPEND)
	{
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
	else
	{
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, (unsigned char*) output.c_str(), sizeof(output)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
                u4_t netid = 0;
                devaddr_t devaddr = 0;
                u1_t nwkKey[16];
                u1_t artKey[16];
                LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
                Serial.print("netid: ");
                Serial.println(netid, DEC);
                Serial.print("devaddr: ");
                Serial.println(devaddr, HEX);
                Serial.print("AppSKey: ");
                for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                    Serial.print("-");
                printHex2(artKey[i]);
                }
                Serial.println("");
                Serial.print("NwkSKey: ");
                for (size_t i=0; i<sizeof(nwkKey); ++i) {
                        if (i != 0)
                                Serial.print("-");
                        printHex2(nwkKey[i]);
                }
                Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
                Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
                Serial.println(F("Received "));
                Serial.println(LMIC.dataLen);
                Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            flag_TXCOMPLETE = true;

            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            //ESP.restart();
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.println(F("Starting"));

	phecda.activatePH();
	phecda.activateTEMP();
	phecda.activateORP();

    delay(100);

	phecda.begin();

    delay(100);

    // LMIC init
	Serial.println("OS init");
    os_init();
    //LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
    // Reset the MAC state. Session and pending data transfers will be discarded.
	Serial.println("lmic reset");
	LMIC_reset();


	Serial.println("lmic config");
    //LMIC.dn2Dr = US915_DR_SF12CR;
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
    //LMIC_setLinkCheckMode(0);
    //LMIC_setDrTxpow(DR_SF7,14);
    LMIC_selectSubBand(1);
	
    Serial.println("asing send job");
    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
	Serial.println("ready");    
}

void loop()
{   
    os_runloop_once();

    //if (flag_TXCOMPLETE) {
    //    delay(10000);
    //    // Entrar en el modo de deep sleep
    //    Serial.println("Deep Sleep"); 
    //    Serial.flush();
    //    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    //    esp_deep_sleep_start();
    //}
}
