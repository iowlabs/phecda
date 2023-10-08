#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define S_A 0x3C //< See datasheet for Address;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup(){
	Serial.begin(115200);
	display.begin(S_A,true);
	if(!display.begin(S_A,true)){
		Serial.println("SH110X allocation failed");
		for(;;);
	}
	display.setRotation(2); //IF NOT INVERTED COMMENT THIS LINE
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.display(); // Initialize with display cleared
	display.clearDisplay(); // Clear the buffer
}
void loop(){

  Serial.println("imprimiendo");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("pH:");
  display.setTextSize(3);
  display.setCursor(30, 22);
  display.print("7.0");
  display.display();
  delay(200);
}
