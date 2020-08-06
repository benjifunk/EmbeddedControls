#include "Memory.h"
#include <EEPROM\src\EEPROM.h>
#include "shiftlight.h"

void(*resetFunc) (void) = 0;


void Memory::writeMilesEEPROM() {                     // This is the subroutine that is called when interrupt 1 goes low
										 // store mileage variables in EEPROM
	EEPROM.update(0, byte((distanceMiles & 65280) / 256));
	EEPROM.update(1, byte((distanceMiles & 255)));
	EEPROM.update(2, byte(distanceTenths));
	EEPROM.update(3, byte((distanceClicks) & 65280) / 256);
	EEPROM.update(4, byte((distanceClicks) & 255));
	EEPROM.update(5, byte((totalMiles & 16711680) / 65536));
	EEPROM.update(6, byte((totalMiles & 65280) / 256));
	EEPROM.update(7, byte((totalMiles & 255)));
	EEPROM.update(8, byte(totalTenths));
	EEPROM.update(9, byte((totalClicks) & 65280) / 256);
	EEPROM.update(10, byte((totalClicks) & 255));
}


//This subroutine writes the stored variables to memory
void Memory::writeSettingsEEPROM() {

	byte four = (shift_rpm & 0xFF);
	byte three = ((shift_rpm >> 8) & 0xFF);
	byte two = ((shift_rpm >> 16) & 0xFF);
	byte one = ((shift_rpm >> 24) & 0xFF);

	byte activation_four = (activation_rpm & 0xFF);
	byte activation_three = ((activation_rpm >> 8) & 0xFF);
	byte activation_two = ((activation_rpm >> 16) & 0xFF);
	byte activation_one = ((activation_rpm >> 24) & 0xFF);

	EEPROM.update(11, shiftlight.brightval);
	EEPROM.update(12, sb);
	EEPROM.update(13, c1);
	EEPROM.update(14, c2);
	EEPROM.update(15, c3);
	EEPROM.update(16, c4);
	EEPROM.update(17, c5);
	EEPROM.update(18, activation_rpm);
	EEPROM.update(19, pixelanim);
	EEPROM.update(20, senseoption);
	EEPROM.update(21, smoothing);
	EEPROM.write(22, NUMPIXELS);
	EEPROM.update(23, rpmscaler);
	EEPROM.update(24, four);
	EEPROM.update(25, three);
	EEPROM.update(26, two);
	EEPROM.update(27, one);
	EEPROM.update(28, DEBUG);
	EEPROM.update(29, seg1_start);
	EEPROM.update(30, seg1_end);
	EEPROM.update(31, seg2_start);
	EEPROM.update(32, seg2_end);
	EEPROM.update(33, seg3_start);
	EEPROM.update(34, seg3_end);
	EEPROM.update(35, activation_four);
	EEPROM.update(36, activation_three);
	EEPROM.update(37, activation_two);
	EEPROM.update(38, activation_one);
	EEPROM.update(39, cal);
}

//Helper Color Manager - This translates our 255 value into a meaningful color
uint32_t Memory::load_color(int cx) {
	unsigned int r, g, b;
	if (cx == 0)
	{
		r = 0;
		g = 0;
		b = 0;
	}

	if (cx > 0 && cx <= 85)
	{
		r = 255 - (cx * 3);
		g = cx * 3;
		b = 0;
	}

	if (cx > 85 && cx < 170)
	{
		r = 0;
		g = 255 - ((cx - 85) * 3);
		b = (cx - 85) * 3;
	}

	if (cx >= 170 && cx < 255)
	{
		r = (cx - 170) * 3;
		g = 0;
		b = 255 - ((cx - 170) * 3);
	}

	if (cx == 255)
	{
		r = 255;
		g = 255;
		b = 255;
	}

	r = (r / sb);
	g = (g / sb);
	b = (b / sb);
	return strip.Color(r, g, b);
}

void Memory::check_first_run() {
	if (shift_rpm == 0)
	{
		Serial.println("FIRST RUN! LOADING DEFAULTS");
		brightval = 0;
		sb = 15;
		c1 = 79;
		c2 = 48;
		c3 = 1;
		c4 = 255;
		c5 = 0;

		activation_rpm = 1000;
		shift_rpm = 6000;
		pixelanim = 1;
		senseoption = 2;
		smoothing = 1;
		NUMPIXELS = 8;
		//rpmscaler = EEPROM.read(12);
		DEBUG = 0;
		seg1_start = 0;
		seg1_end = 3;
		seg2_start = 0;
		seg2_end = 5;
		seg3_start = 0;
		seg3_end = 7;
		cal = 1;
		writeSettingsEEPROM();
		resetFunc();
	}
}

void Memory::build_segments() {

	if (pixelanim == 3)
	{
		seg_mover = NUMPIXELS - 1;
	}

	while (current_seg_number < 4)
	{
		tftdrawNum(current_seg_number);


		if (leftButton.uniquePress())
		{
			seg_mover--;
		}
		if (rightButton.uniquePress())
		{
			seg_mover++;
		}

		if (menuButton.uniquePress())
		{
			delay(250);
			current_seg_number++;
		}

		switch (current_seg_number)
		{
		case 1:
			if (pixelanim == 1)
			{
				seg_mover = constrain(seg_mover, 0, (NUMPIXELS - 1));
				seg1_end = seg_mover;
				strip.setPixelColor(seg1_end, color1);
				for (int x = seg1_end + 1; x < NUMPIXELS; x++)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}
			}
			else if (pixelanim == 3)
			{
				seg_mover = constrain(seg_mover, 0, (NUMPIXELS - 1));
				seg1_start = seg_mover;
				strip.setPixelColor(seg1_start, color1);
				for (int x = seg1_start - 1; x > -1; x--)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}
			}
			else if (pixelanim == 2)
			{
				seg1_start = ((NUMPIXELS - 1) / 2);
				if (((NUMPIXELS - 1) % 2) > 0)
				{
					seg1_start = seg1_start + 1;
				}
				seg_mover = constrain(seg_mover, seg1_start, (NUMPIXELS - 1));
				seg1_end = seg_mover;

				for (int x = seg1_start; x < seg1_end + 1; x++)
				{
					strip.setPixelColor(x, color1);
				}
				for (int x = seg1_end + 1; x < (NUMPIXELS); x++)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}

				if (((NUMPIXELS - 1) % 2) > 0)
				{
					for (int x = seg1_start - 1; x > seg1_start - (seg1_end - seg1_start) - 2; x--)
					{
						strip.setPixelColor(x, color1);
					}
					for (int x = seg1_start - (seg1_end - seg1_start) - 2; x > -1; x--)
					{
						strip.setPixelColor(x, strip.Color(0, 0, 0));
					}
					if (DEBUG)
					{
						Serial.println("MoDULo");
					}
				}
				else
				{
					for (int x = seg1_start; x > seg1_start - (seg1_end - seg1_start) - 1; x--)
					{
						strip.setPixelColor(x, color1);
					}
					for (int x = seg1_start - (seg1_end - seg1_start) - 1; x > -1; x--)
					{
						strip.setPixelColor(x, strip.Color(0, 0, 0));
					}
				}
			}
			if (DEBUG)
			{
				Serial.print("S1end: ");
				Serial.println(seg1_end);
				Serial.print("S1start: ");
				Serial.println(seg1_start);
			}

			strip.show();


			break;



		case 2:
			if (pixelanim == 1)
			{
				seg_mover = constrain(seg_mover, seg1_end + 1, (NUMPIXELS - 1));
				seg2_end = seg_mover;
				strip.setPixelColor(seg2_end, color2);
				for (int x = seg2_end + 1; x < strip.numPixels(); x++)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}
			}
			else if (pixelanim == 3)
			{
				seg_mover = constrain(seg_mover, 0, (seg1_start - 1));
				seg2_start = seg_mover;
				strip.setPixelColor(seg2_start, color2);
				for (int x = seg2_start - 1; x > -1; x--)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}
			}
			else if (pixelanim == 2)
			{ //seg1_start = ((NUMPIXELS-1)/2);
				seg2_start = seg1_end + 1;
				//  if (((NUMPIXELS-1)%2)> 0){seg1_start=seg1_start+1;}
				seg_mover = constrain(seg_mover, seg2_start, (NUMPIXELS - 1));
				seg2_end = seg_mover;

				for (int x = seg2_start; x < seg2_end + 1; x++)
				{
					strip.setPixelColor(x, color2);
				}
				for (int x = seg2_end + 1; x < (NUMPIXELS); x++)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}

				if (((NUMPIXELS - 1) % 2) > 0)
				{
					for (int x = seg1_start - (seg1_end - seg1_start) - 2; x > seg1_start - (seg2_end - seg1_start) - 2; x--)
					{
						strip.setPixelColor(x, color2);
					}
					for (int x = seg1_start - (seg2_end - seg1_start) - 2; x > -1; x--)
					{
						strip.setPixelColor(x, strip.Color(0, 0, 0));
					}
					if (DEBUG)
					{
						Serial.println("MoDULo");
					}
				}
				else
				{
					for (int x = seg1_start - (seg1_end - seg1_start) - 1; x > seg1_start - (seg2_end - seg1_start) - 1; x--)
					{
						strip.setPixelColor(x, color2);
					}
					for (int x = seg1_start - (seg2_end - seg1_start) - 1; x > -1; x--)
					{
						strip.setPixelColor(x, strip.Color(0, 0, 0));
					}
				}

			}

			if (DEBUG)
			{
				Serial.print("S2end: ");
				Serial.println(seg2_end);
				Serial.print("S2start: ");
				Serial.println(seg2_start);
			}

			strip.show();
			break;

		case 3:

			if (pixelanim == 1)
			{
				seg_mover = constrain(seg_mover, seg2_end + 1, (strip.numPixels() - 1));
				seg3_end = seg_mover;
				// seg3_start = seg2_end +1;
				strip.setPixelColor(seg3_end, color3);
				for (int x = seg3_end + 1; x < strip.numPixels(); x++)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}
			}
			else if (pixelanim == 3)
			{
				seg_mover = constrain(seg_mover, 0, (seg2_start - 1));
				seg3_start = seg_mover;
				strip.setPixelColor(seg3_start, color3);
				for (int x = seg3_start - 1; x > -1; x--)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}
			}
			else if (pixelanim == 2)
			{
				seg3_start = seg2_end + 1;
				seg_mover = constrain(seg_mover, seg3_start, (NUMPIXELS - 1));
				seg3_end = seg_mover;

				for (int x = seg3_start; x < seg3_end + 1; x++)
				{
					strip.setPixelColor(x, color3);
				}
				for (int x = seg3_end + 1; x < (NUMPIXELS); x++)
				{
					strip.setPixelColor(x, strip.Color(0, 0, 0));
				}

				if (((NUMPIXELS - 1) % 2) > 0)
				{
					for (int x = seg1_start - (seg2_end - seg1_start) - 2; x > seg1_start - (seg3_end - seg1_start) - 2; x--)
					{
						strip.setPixelColor(x, color3);
					}
					for (int x = seg1_start - (seg3_end - seg1_start) - 2; x > -1; x--)
					{
						strip.setPixelColor(x, strip.Color(0, 0, 0));
					}
					if (DEBUG)
					{
						Serial.println("MoDULo");
					}
				}
				else
				{
					for (int x = seg1_start - (seg2_end - seg1_start) - 1; x > seg1_start - (seg3_end - seg1_start) - 1; x--)
					{
						strip.setPixelColor(x, color3);
					}
					for (int x = seg1_start - (seg3_end - seg1_start) - 1; x > -1; x--)
					{
						strip.setPixelColor(x, strip.Color(0, 0, 0));
					}
				}
			}

			if (DEBUG)
			{
				Serial.print("S3end: ");
				Serial.println(seg3_end);
				Serial.print("S3start: ");
				Serial.println(seg3_start);
			}
			strip.show();
			break;




		}
	}
}
