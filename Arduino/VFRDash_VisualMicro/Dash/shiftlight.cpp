
#include "shiftlight.h"


void(*resetFunc) (void) = 0;

	extern uint8_t DEBUG;
	extern int NUMPIXELS;

	const int sensorInterrupt = 0;
	const int timeoutValue = 10;
	volatile unsigned long lastPulseTime;
	volatile unsigned long interval = 0;
	volatile int timeoutCounter;
	long rpm;
	long rpm_last;
	int display_rpm;
	int activation_rpm;
	int shift_rpm;
	int menu_enter = 0;
	int current_seg_number = 1;
	int seg_mover = 0;
	long previousMillis = 0;
	int shiftinterval = 50;
	boolean flashbool = true;
	int prev_animation;
	int prev_color;
	boolean testbright = false;
	int prev_menu;
	boolean testdim = false;
	int justfixed;

	//array for rpm averaging, filtering comparison
	const int numReadings = 5;
	int rpmarray[numReadings];
	int index = 0;                  // the index of the current reading
	long total = 0;                  // the running total
	long average = 0;                // the average


	//These are stored memory variables for adjusting the (5) colors, activation rpm, shift rpm, brightness 
	//Stored in EEPROM Memory 
	int c1;
	int c2;
	int c3;
	int c4;
	int c5;
	int brightval;      //7-seg brightness 
	int dimval;         //7-seg dim brightness
	int sb;             //strip brightness
	int dimsb;          // strip dim brightness
	boolean dimmer = false;
	boolean dimmerlogic = false;
	int pixelanim = 1;
	int smoothing;
	int rpmscaler;
	int shift_rpm1;                          // USED TO BE A LONG!!
	int shift_rpm2;
	int shift_rpm3;
	int shift_rpm4;
	int seg1_start = 1;
	int seg1_end = 1;
	int seg2_end = 2;
	int seg3_end = 3;
	int activation_rpm1;
	int activation_rpm2;
	int activation_rpm3;
	int activation_rpm4;
	int a;

	int rst = 0;
	int cal;

	int prev_cal;

	// COLOR VARIABLES - for use w/ the strips and translated into 255 RGB colors 
	long color1;
	long color2;
	long color3;
	long flclr1;


//Creates a 32 wide table for our pixel animations
int rpmtable[32][2];

 Adafruit_DotStar strip = Adafruit_DotStar(EEPROM.read(11), DATAPIN, CLOCKPIN, DOTSTAR_BGR);

Shiftlight::Shiftlight()
{
	attachInterrupt(0, sensorIsr, RISING);
	loadallcolors();

	bootanimation();
	delay(1000);
	if(DEBUG){Serial.println("LOADED.");}
}

void loadallcolors() {
	color1 = load_color(c1);
	color2 = load_color(c2);
	color3 = load_color(c3);
	flclr1 = load_color(c4);
}



//Helper Color Manager - This translates our 255 value into a meaningful color
uint32_t load_color(int cx) {
	unsigned int r, g, b;
	if (cx == 0) {
		r = 0;
		g = 0;
		b = 0;
	}

	if (cx>0 && cx <= 85) {
		r = 255 - (cx * 3);
		g = cx * 3;
		b = 0;
	}

	if (cx>85 && cx < 170) {
		r = 0;
		g = 255 - ((cx - 85) * 3);
		b = (cx - 85) * 3;
	}

	if (cx >= 170 && cx<255) {
		r = (cx - 170) * 3;
		g = 0;
		b = 255 - ((cx - 170) * 3);
	}

	if (cx == 255) {
		r = 255;
		g = 255;
		b = 255;
	}



	if (digitalRead(dimPin) == dimmerlogic || testdim == true) {
		r = (r / dimsb);
		g = (g / dimsb);
		b = (b / dimsb);
	}
	else {
		r = (r / sb);
		g = (g / sb);
		b = (b / sb);
	}

	return strip.Color(r, g, b);
}



void Shiftlight::testlights() {
	for (int a = 0; a<NUMPIXELS; a++) {
		switch (rpmtable[a][1]) {
		case 1:
			strip.setPixelColor(a, color1);
			break;

		case 2:
			strip.setPixelColor(a, color2);
			break;

		case 3:
			strip.setPixelColor(a, color3);
			break;
		}

	}
	strip.show();
}

//This subroutine reads the stored variables from memory 
void Shiftlight::getEEPROM() {
	brightval = EEPROM.read(0);
	sb = EEPROM.read(1);
	c1 = EEPROM.read(2);
	c2 = EEPROM.read(3);
	c3 = EEPROM.read(4);
	c4 = EEPROM.read(5);
	c5 = EEPROM.read(6);
	activation_rpm = EEPROM.read(7);
	pixelanim = EEPROM.read(8);
	//senseoption  = EEPROM.read(9); 
	smoothing = EEPROM.read(10);
	NUMPIXELS = EEPROM.read(11);
	rpmscaler = EEPROM.read(12);
	shift_rpm1 = EEPROM.read(13);
	shift_rpm2 = EEPROM.read(14);
	shift_rpm3 = EEPROM.read(15);
	shift_rpm4 = EEPROM.read(16);
	DEBUG = EEPROM.read(17);
	seg1_start = EEPROM.read(18);
	seg1_end = EEPROM.read(19);
	//seg2_start = EEPROM.read(20); 
	seg2_end = EEPROM.read(21);
	//seg3_start = EEPROM.read(22); 
	seg3_end = EEPROM.read(23);
	activation_rpm1 = EEPROM.read(24);
	activation_rpm2 = EEPROM.read(25);
	activation_rpm3 = EEPROM.read(26);
	activation_rpm4 = EEPROM.read(27);
	cal = EEPROM.read(28);
	dimval = EEPROM.read(29);
	dimsb = EEPROM.read(30);
	dimmerlogic = EEPROM.read(31);

	activation_rpm = ((activation_rpm1 << 0) & 0xFF) + ((activation_rpm2 << 8) & 0xFFFF) + ((activation_rpm3 << 16) & 0xFFFFFF) + ((activation_rpm4 << 24) & 0xFFFFFFFF);
	shift_rpm = ((shift_rpm1 << 0) & 0xFF) + ((shift_rpm2 << 8) & 0xFFFF) + ((shift_rpm3 << 16) & 0xFFFFFF) + ((shift_rpm4 << 24) & 0xFFFFFFFF);

	buildarrays();

}

//This subroutine writes the stored variables to memory 
void Shiftlight::writeEEPROM() {

	byte four = (shift_rpm & 0xFF);
	byte three = ((shift_rpm >> 8) & 0xFF);
	byte two = ((shift_rpm >> 16) & 0xFF);
	byte one = ((shift_rpm >> 24) & 0xFF);

	byte activation_four = (activation_rpm & 0xFF);
	byte activation_three = ((activation_rpm >> 8) & 0xFF);
	byte activation_two = ((activation_rpm >> 16) & 0xFF);
	byte activation_one = ((activation_rpm >> 24) & 0xFF);

	EEPROM.write(0, brightval);
	EEPROM.write(1, sb);
	EEPROM.write(2, c1);
	EEPROM.write(3, c2);
	EEPROM.write(4, c3);
	EEPROM.write(5, c4);
	EEPROM.write(6, c5);
	EEPROM.write(7, activation_rpm);
	EEPROM.write(8, pixelanim);
	//EEPROM.write(9, senseoption); 
	EEPROM.write(10, smoothing);
	EEPROM.write(11, NUMPIXELS);
	EEPROM.write(12, rpmscaler);
	EEPROM.write(13, four);
	EEPROM.write(14, three);
	EEPROM.write(15, two);
	EEPROM.write(16, one);
	EEPROM.write(17, DEBUG);
	EEPROM.write(18, seg1_start);
	EEPROM.write(19, seg1_end);
	//EEPROM.write(20, seg2_start); 
	EEPROM.write(21, seg2_end);
	//EEPROM.write(22, seg3_start); 
	EEPROM.write(23, seg3_end);
	EEPROM.write(24, activation_four);
	EEPROM.write(25, activation_three);
	EEPROM.write(26, activation_two);
	EEPROM.write(27, activation_one);
	EEPROM.write(28, cal);
	EEPROM.write(29, dimval);
	EEPROM.write(30, dimsb);
	EEPROM.write(31, dimmerlogic);
}

void Shiftlight::build_segments() {

// Display to user what the function does : " rotate encoder to change width, press button to convirm" etc
	//
	//
	//


	// Resets segmentation variables, sets segments 2 and 4 outside of the range
	int prev_seg_mover = -1;
	current_seg_number = 1;
	seg1_start = 0;
	seg1_end = 0;
	seg2_end = NUMPIXELS + 1;
	seg3_end = NUMPIXELS + 1;

	// Based on the animation, we must reconfigure some segmentation variables to known limits  
	switch (pixelanim) {
	case 1:
		seg_mover = 0;
		break;

	case 2:

		seg1_start = (NUMPIXELS / 2);
		seg_mover = (NUMPIXELS / 2);
		break;

	case 3:
		seg_mover = NUMPIXELS - 1;
		seg1_end = NUMPIXELS - 1;
		seg2_end = 0;
		seg3_end = 0;
		break;

	case 4:
		seg_mover = 0;
		if (((NUMPIXELS - 1) % 2)> 0) {
			seg2_end = (NUMPIXELS / 2) - 1;
		}
		else {
			seg2_end = NUMPIXELS / 2;
		};
		if (((NUMPIXELS - 1) % 2)> 0) {
			seg3_end = (NUMPIXELS / 2) - 1;
		}
		else {
			seg3_end = NUMPIXELS / 2;
		};
		break;
	}


	while (current_seg_number<3) {
		int colorsegmenter = rotary_process();
		if (colorsegmenter == -128) { seg_mover--; }
		if (colorsegmenter == 64) { seg_mover++; }

		switch (pixelanim) {
		case 1:
			switch (current_seg_number) {
			case 1:
				seg_mover = constrain(seg_mover, 0, NUMPIXELS - 1);
				break;
			case 2:
				seg_mover = constrain(seg_mover, seg1_end + 1, NUMPIXELS - 1);
				break;
			}

			break;

		case 2:
			switch (current_seg_number) {
			case 1:
				seg_mover = constrain(seg_mover, NUMPIXELS / 2, NUMPIXELS - 1);
				break;
			case 2:
				seg_mover = constrain(seg_mover, seg1_end + 1, NUMPIXELS - 1);
				break;
			}
			break;

		case 3:
			switch (current_seg_number) {
			case 1:
				seg_mover = constrain(seg_mover, 0, NUMPIXELS - 1);
				break;
			case 2:
				seg_mover = constrain(seg_mover, 0, seg1_end - 1);
				break;
			}

			break;

		case 4:
			switch (current_seg_number) {
			case 1:
				if (((NUMPIXELS - 1) % 2)> 0) {
					seg_mover = constrain(seg_mover, 0, (NUMPIXELS / 2) - 2);
				}
				else {
					seg_mover = constrain(seg_mover, 0, (NUMPIXELS / 2) - 1);
				}
				break;
			case 2:
				if (((NUMPIXELS - 1) % 2)> 0) {
					seg_mover = constrain(seg_mover, seg1_end + 1, (NUMPIXELS / 2) - 1);
				}
				else {
					seg_mover = constrain(seg_mover, seg1_end + 1, NUMPIXELS / 2);
				}
				break;
			}
			break;
		}


		if (digitalRead(button_pin) == LOW) {
			delay(250);
			current_seg_number++;
		}


		if (prev_seg_mover != seg_mover) {
			prev_seg_mover = seg_mover;

			switch (current_seg_number) {
			case 1:
				seg1_end = seg_mover;
				break;

			case 2:
				seg2_end = seg_mover;
				break;

			}
			buildarrays();
			loadallcolors();
			testlights();
		}
	}
}

void Shiftlight::buildarrays() {

	int x;  //rpm increment
	int y;  //starting point pixel address
	int ya; // second starting point pixel address (for middle-out animation only)
	int i;  //temporary for loop variable

	if (DEBUG) {
		Serial.print("NUMPIXELS:  ");
		Serial.println(NUMPIXELS);
		Serial.print("PIXELANIM:  ");
		Serial.println(pixelanim);
		Serial.print("Start1: ");
		Serial.println(seg1_start);
		Serial.print("End1: ");
		Serial.println(seg1_end);
		Serial.print("End2: ");
		Serial.println(seg2_end);
		Serial.print("End3: ");
		Serial.println(seg3_end);
		Serial.print("  Activation RPM ");
		Serial.println(activation_rpm);
		Serial.print("  SHIFT RPM ");
		Serial.println(shift_rpm);
	}

	switch (pixelanim) {

	case 1:
		y = 0;
		x = ((shift_rpm - activation_rpm) / NUMPIXELS);
		for (i = 0; i<seg1_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (i*x);
			rpmtable[i][1] = 1;
		}
		for (i = seg1_end + 1; i<seg2_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (i*x);
			rpmtable[i][1] = 2;
		}
		for (i = seg2_end + 1; i<seg3_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (i*x);
			rpmtable[i][1] = 3;
		}
		break;


	case 2:
		if (((NUMPIXELS - 1) % 2)> 0) {
			x = ((shift_rpm - activation_rpm) / (NUMPIXELS / 2));  //EVEN PIXELS
		}
		else {
			x = ((shift_rpm - activation_rpm) / ((NUMPIXELS / 2) + 1));   //ODD PIXELS       
		}


		ya = 0;   // SEGMENT 1
		for (i = seg1_start; i<seg1_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (ya*x);
			rpmtable[i][1] = 1;
			ya++;
		}


		if (((NUMPIXELS - 1) % 2)> 0) {
			ya = 0;
			for (i = seg1_start - 1; i>seg1_start - (seg1_end - seg1_start) - 2; i--) {
				rpmtable[i][0] = activation_rpm + (ya*x);
				rpmtable[i][1] = 1;
				ya++;
			}
		}
		else {
			ya = 1;
			for (i = seg1_start - 1; i>seg1_start - (seg1_end - seg1_start) - 1; i--) {
				rpmtable[i][0] = activation_rpm + (ya*x);
				rpmtable[i][1] = 1;
				ya++;
			}
		}



		if ((seg1_end + 1) == seg2_end) {
			ya = seg2_end - seg1_start;  //SEGMENT 2
		}
		else {
			ya = (seg1_end + 1) - seg1_start;
		}

		for (i = (seg1_end + 1); i<seg2_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (ya*x);
			rpmtable[i][1] = 2;
			ya++;
		}

		if ((seg1_end + 1) == seg2_end) {
			ya = seg2_end - seg1_start;  //SEGMENT 2
		}
		else {
			ya = (seg1_end + 1) - seg1_start;
		}

		if (((NUMPIXELS - 1) % 2)> 0) {
			for (i = seg1_start - (seg1_end - seg1_start) - 2; i>seg1_start - (seg2_end - seg1_start) - 2; i--) {
				rpmtable[i][0] = activation_rpm + (ya*x);
				rpmtable[i][1] = 2;
				ya++;
			}
		}
		else {
			for (i = seg1_start - (seg1_end - seg1_start) - 1; i>seg1_start - (seg2_end - seg1_start) - 1; i--) {
				rpmtable[i][0] = activation_rpm + (ya*x);
				rpmtable[i][1] = 2;
				ya++;
			}
		}

		if ((seg2_end + 1) == seg3_end) {
			ya = seg3_end - seg1_start;    //SEGMENT 3
		}
		else {
			ya = (seg2_end + 1) - seg1_start;    //SEGMENT 3  
		}


		for (i = (seg2_end + 1); i<seg3_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (ya*x);
			rpmtable[i][1] = 3;
			ya++;
		}

		if ((seg2_end + 1) == seg3_end) {
			ya = seg3_end - seg1_start;
		}
		else {
			ya = (seg2_end + 1) - seg1_start;
		}

		if (((NUMPIXELS - 1) % 2)> 0) {
			for (i = seg1_start - (seg2_end - seg1_start) - 2; i>seg1_start - (seg3_end - seg1_start) - 2; i--) {
				rpmtable[i][0] = activation_rpm + (ya*x);
				rpmtable[i][1] = 3;
				ya++;
			}
		}
		else {
			for (i = seg1_start - (seg2_end - seg1_start) - 1; i>seg1_start - (seg3_end - seg1_start) - 1; i--) {
				rpmtable[i][0] = activation_rpm + (ya*x);
				rpmtable[i][1] = 3;
				ya++;
			}
		}
		break;

	case 3:
		y = 0;
		x = ((shift_rpm - activation_rpm) / NUMPIXELS);
		for (i = NUMPIXELS - 1; i>seg1_end - 1; i--) {
			rpmtable[i][0] = activation_rpm + (y*x);
			rpmtable[i][1] = 1;
			y++;
		}
		for (i = seg1_end - 1; i>seg2_end - 1; i--) {
			rpmtable[i][0] = activation_rpm + (y*x);
			rpmtable[i][1] = 2;
			y++;
		}
		for (i = seg2_end - 1; i>seg3_end - 1; i--) {
			rpmtable[i][0] = activation_rpm + (y*x);
			rpmtable[i][1] = 3;
			y++;
		}
		break;

	case 4:
		if (((NUMPIXELS - 1) % 2)> 0) {
			x = ((shift_rpm - activation_rpm) / (NUMPIXELS / 2));  //EVEN PIXELS
		}
		else {
			x = ((shift_rpm - activation_rpm) / ((NUMPIXELS / 2) + 1));   //ODD PIXELS       
		}


		// SEGMENT 1
		for (i = seg1_start; i<seg1_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (i*x);
			rpmtable[i][1] = 1;
		}

		ya = 0;
		for (i = NUMPIXELS - 1; i>NUMPIXELS - (seg1_end)-2; i--) {
			rpmtable[i][0] = activation_rpm + (ya*x);
			rpmtable[i][1] = 1;
			ya++;
		}

		// SEGMENT 2
		for (i = (seg1_end + 1); i<seg2_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (i*x);
			rpmtable[i][1] = 2;
		}


		ya = seg1_end + 1;
		for (i = NUMPIXELS - (seg1_end)-2; i>NUMPIXELS - seg2_end - 2; i--) {
			rpmtable[i][0] = activation_rpm + (ya*x);
			rpmtable[i][1] = 2;
			ya++;
		}

		// SEGMENT 3
		for (i = (seg2_end + 1); i<seg3_end + 1; i++) {
			rpmtable[i][0] = activation_rpm + (i*x);
			rpmtable[i][1] = 3;
		}

		ya = seg2_end + 1;
		for (i = NUMPIXELS - (seg2_end)-2; i>NUMPIXELS - seg3_end - 2; i--) {
			rpmtable[i][0] = activation_rpm + (ya*x);
			rpmtable[i][1] = 3;
			ya++;
		}

		break;

	}

	if (DEBUG) {
		for (i = 0; i<NUMPIXELS; i++) {
			Serial.print(rpmtable[i][0]);
			Serial.print("  ");
			Serial.println(rpmtable[i][1]);
		}
	}
}

void sensorIsr()
{
	unsigned long now = micros();
	interval = now - lastPulseTime;
	lastPulseTime = now;
	timeoutCounter = timeoutValue;
}

void Shiftlight::check_first_run() {

	if (shift_rpm == 0) {
		Serial.println("FIRST RUN! LOADING DEFAULTS");
		brightval = 15;
		dimval = 8;
		dimsb = 15;
		dimmerlogic = false;
		sb = 3;
		c1 = 79;
		c2 = 48;
		c3 = 1;
		c4 = 255;
		c5 = 0;

		activation_rpm = 1000;
		shift_rpm = 6000;
		pixelanim = 1;
		smoothing = 0;
		NUMPIXELS = 16;
		//rpmscaler = EEPROM.read(12);  
		DEBUG = 1;
		seg1_start = 0;
		seg1_end = 10;
		seg2_end = 13;
		seg3_end = 15;
		cal = 30;
		writeEEPROM();
		resetFunc();
	}
}

void Shiftlight::menuExitAnim() {
	strip.clear();
	strip.show();
	for (int i = 0; i<NUMPIXELS + 1; i++) {
		strip.setPixelColor(i, strip.Color(50, 50, 50));
		strip.show();
		delay(15);
		strip.setPixelColor(i, strip.Color(0, 0, 0));
		strip.show();
	}
}

void Shiftlight::menuEnterAnim() {
	//Ascend strip 
	for (int i = 0; i<(NUMPIXELS / 2) + 1; i++) {
		strip.setPixelColor(i, strip.Color(0, 0, 25));
		strip.setPixelColor(NUMPIXELS - i, strip.Color(0, 0, 25));
		strip.show();
		delay(35);
	}
	// Descend Strip 
	for (int i = 0; i<(NUMPIXELS / 2) + 1; i++) {
		strip.setPixelColor(i, strip.Color(0, 0, 0));
		strip.setPixelColor(NUMPIXELS - i, strip.Color(0, 0, 0));
		strip.show();
		delay(35);
	}
}

void Shiftlight::bootanimation() {
	int colorwidth;
	colorwidth = (50 / (NUMPIXELS / 2));
	for (int i = 0; i<(NUMPIXELS / 2) + 1; i++) {
		strip.setPixelColor(i, strip.Color((i*colorwidth), (i*colorwidth), ((25 * (NUMPIXELS / 2)) - ((25 * (i / 2)) + 1))));
		strip.setPixelColor(NUMPIXELS - i, strip.Color((i*colorwidth), (i*colorwidth), ((25 * (NUMPIXELS / 2)) - ((25 * (i / 2)) + 1))));
		strip.show();
		delay(25);
		strip.setPixelColor(i - 1, strip.Color(0, 0, 0));
		strip.setPixelColor(NUMPIXELS - i + 1, strip.Color(0, 0, 0));
		strip.show();
	}

	for (int i = 35; i>0; i--) {
		delay(15);
		strip.setPixelColor((NUMPIXELS / 2), strip.Color(i, i, i));
		strip.show();
	}

	strip.clear();
	strip.show();
}