
#include "display.h"
#include <Adafruit_ILI9341\Adafruit_ILI9341.h>
#include <SPI.h>



void display::start() {
	tft.begin();
	tft.setRotation(0);
	tft.fillScreen(ILI9341_BLACK);
}

void display::setFont(uint8_t txt, uint8_t bkg, int size) {
	tft.setTextColor(txt, bkg);
	tft.setTextSize(size);
}

void display::prints(char* s) {
	tft.drawString(s, xpos, ypos, font);
}

void display::reprints(char* s) {
	tft.fillScreen(ILI9341_BLACK);
	tft.drawString(s, xpos, ypos, font);
}

void display::setPosition(int x, int y) {
	xpos = x;
	ypos = y;
}

void display::setFont(int t) {
	font = t;
}

void display::clear() {
	tft.fillScreen(ILI9341_BLACK);
}
