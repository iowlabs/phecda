#include <Adafruit_SH110X.h>
#include "phecda.h"


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define S_A 0x3C //< See datasheet for Address;



Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
	Serial.begin(115200);
	display.begin(S_A,true);

	display.setRotation(2); //IF NOT INVERTED COMMENT THIS LINE
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.display(); // Initialize with display cleared
	display.clearDisplay(); // Clear the buffer
  display.drawBitmap(0, 0, logo_iowlabs, 128, 64,1,0);
  display.display();
  delay(1000);
  display.drawBitmap(0, 0, logo_phecda, 128, 64,1,0);
  display.display();

}
void loop(){


}
