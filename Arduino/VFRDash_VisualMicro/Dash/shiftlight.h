/* 
This header file contains functions specific to the neopixel strip light.
*/
#ifndef SHIFTLIGHT_H
#define SHIFTLIGHT_H

#include <Adafruit_DotStar\Adafruit_DotStar.h>
#include "Pins.h"
#include "display.h"
#include "EEPROM\src\EEPROM.h"

class Shiftlight
{
public:
	Shiftlight();
	void writeEEPROM();
	void getEEPROM();
	void bootanimation();
	void menuEnterAnim();
	void menuExitAnim();
	void testlights();
	void check_first_run();
	void build_segments();
	void buildarrays();
};

#endif