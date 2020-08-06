#include "menu.h"

int prev_variable;

Encoder enc = Encoder();

// MENU SYSTEM 
void Menu::enter() {
	prev_menu = 2;
	//this keeps us in the menu 
	while (menuvar == 1) {

		// This little bit calls the rotary encoder   
		int result = enc.rotary_process();
		if (DEBUG) { if (result != 0) { Serial.println(result); } }
		if (result == -128) { enc.rotaryval--; }
		else if (result == 64) { enc.rotaryval++; }

		rotaryval = constrain(rotaryval, 0, 15);

		//Poll the rotary encoder button to enter menu items
		if (digitalRead(button_pin) == LOW) {
		//	oled.clear();
			delay(250);
			menu_enter = 1;
		}

		if (prev_menu != rotaryval || menu_enter == 1 || menu_enter == 2) {
			prev_menu = rotaryval;
			if (menu_enter == 2) { menu_enter = 0; }

			oled.clear();
			oled.set1X();
			oled.setFont(utf8font10x16);

			switch (rotaryval) {

			case 0: //Menu Screen. Exiting saves variables to EEPROM 

				oled.setCursor(50, 0);
				oled.print(F("MENU"));
				oled.setCursor(0, 2);
				oled.print(F("Press to Save & Exit."));
				oled.setCursor(0, 4);
				oled.print(F(" "));
				oled.setCursor(0, 6);
				oled.print(F("Rotate to browse menu"));

				//Poll the Button to exit 
				if (menu_enter == 1) {
					oled.clear();
					delay(250);
					rotaryval = 0;
					menuvar = 0;
					menu_enter = 0;
					writeEEPROM();
					getEEPROM();
					buildarrays();
					loadallcolors();
					entermenu();
				}
				break;


			case 1: //Adjust the global brightness 

				if (menu_enter == 0) {
					oled.setCursor(30, 0);
					oled.print(F("BRIGHTNESS"));
					oled.setCursor(0, 2);
					oled.print(F("Adjust the global"));
					oled.setCursor(0, 4);
					oled.print(F("brightness level"));
					oled.setCursor(0, 6);
					oled.print(F(""));
				}

				while (menu_enter == 1) {

					int bright = rotary_process();
					if (bright == -128) {
						sb++;
						testbright = false;
					}
					if (bright == 64) {
						sb--;
						testbright = false;
					}

					sb = constrain(sb, 1, 15);

					if (prev_variable != sb) {
						prev_variable = sb;
						loadallcolors();
						oled.set1X();
						oled.setFont(utf8font10x16);
						oled.setCursor(30, 0);
						oled.print(F("BRIGHTNESS"));
						oled.set2X();
						oled.setFont(lcdnums12x16);
						oled.setCursor(0, 2);
						oled.clearToEOL();
						oled.print(16 - sb);

						if (testbright == false) {
							testlights();
							testbright = true;
						}
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						prev_variable = 0;
						exitmenu();
					}
				}
				break;


				//dimPin
			case 2: //Adjust the global brightness DIMMER (when Pin 9 == HIGH)

				if (menu_enter == 0) {
					oled.setCursor(40, 0);
					oled.print(F("DIMMER"));
					oled.setCursor(0, 2);
					oled.print(F("Set dim brightness and"));
					oled.setCursor(0, 4);
					oled.print(F("logic of the dimmer"));
					oled.setCursor(0, 6);
					oled.print(F("wire"));
				}

				while (menu_enter == 1) {

					int bright = rotary_process();

					if (bright == -128) {
						dimsb++;
						testbright = false;
					}
					if (bright == 64) {
						dimsb--;
						testbright = false;
					}

					dimval = map(dimsb, 1, 15, 15, 8);
					dimval = constrain(dimval, 8, 15);
					dimsb = constrain(dimsb, 1, 15);

					if (prev_variable != dimsb) {
						prev_variable = dimsb;
						testdim = true;
						loadallcolors();
						oled.set1X();
						oled.setFont(utf8font10x16);
						oled.setCursor(40, 0);
						oled.print(F("DIMMER"));
						oled.set2X();
						oled.setFont(lcdnums12x16);
						oled.setCursor(0, 2);
						oled.clearToEOL();
						oled.print(16 - dimsb);

						if (testbright == false) {
							testlights();
							testbright = true;
						}
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 4;
						oled.clear();
					}
				}
				while (menu_enter == 4) {

					int bright = rotary_process();

					if (bright == -128) {
						dimmerlogic = false;
						testbright = false;
					}
					if (bright == 64) {
						dimmerlogic = true;
						testbright = false;
					}

					if (prev_variable != dimmerlogic) {
						prev_variable = dimmerlogic;

						if (dimmerlogic == false) {
							oled.set1X();
							oled.setFont(utf8font10x16);
							oled.setCursor(30, 0);
							oled.print(F("DIMMER LOGIC"));
							oled.set2X();
							oled.setFont(utf8font10x16);
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("HIGH"));

						}
						else if (dimmerlogic == true) {

							oled.set1X();
							oled.setFont(utf8font10x16);
							oled.setCursor(30, 0);
							oled.print(F("DIMMER LOGIC"));
							oled.set2X();
							oled.setFont(utf8font10x16);
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("LOW"));
						}

						if (testbright == false) {
							testlights();
							testbright = true;
						}
					}


					if (digitalRead(dimPin) == dimmerlogic) {
						oled.set1X();
						oled.setFont(chippernutdimmer);
						oled.setCursor(108, 0);
						oled.print("c");;                                               // ADd in Flashlight or headlight bulb Icon HERE
					}
					else {
						oled.set1X();
						oled.setFont(chippernutdimmer);
						oled.setCursor(108, 0);
						oled.clearToEOL();
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						prev_variable = 0;
						testdim = false;
						exitmenu();
					}
				}
				break;


			case 3: // ACTIVATION RPM 
				if (menu_enter == 0) {
					oled.setCursor(25, 0);
					oled.print(F("ACTIVATION RPM"));
					oled.setCursor(0, 2);
					oled.print(F("Select the RPM"));
					oled.setCursor(0, 4);
					oled.print(F("value for the start"));
					oled.setCursor(0, 6);
					oled.print(F("of the LED graph."));
				}

				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { activation_rpm = activation_rpm - 10; }
					if (coloradjust1 == 64) { activation_rpm = activation_rpm + 10; }
					activation_rpm = constrain(activation_rpm, 0, 20000);

					if (prev_variable != activation_rpm) {

						oled.set1X();
						oled.setFont(utf8font10x16);
						oled.setCursor(25, 0);
						oled.print(F("ACTIVATION RPM"));
						oled.set2X();
						oled.setFont(lcdnums12x16);
						oled.setCursor(0, 2);
						if ((prev_variable >= 10000 && activation_rpm <10000) ||
							(prev_variable >= 1000 && activation_rpm <1000)) {
							oled.clearToEOL();
						}
						oled.print(activation_rpm);
						prev_variable = activation_rpm;
					}

					if (digitalRead(button_pin) == LOW) {
						delay(250);
						prev_variable = 0;
						menu_enter = 2;
						exitmenu();
					}
				}
				break;


			case 4: // SHIFT RPM 

				if (menu_enter == 0) {
					oled.setCursor(35, 0);
					oled.print(F("SHIFT RPM"));
					oled.setCursor(0, 2);
					oled.print(F("Select the RPM"));
					oled.setCursor(0, 4);
					oled.print(F("that flashes the"));
					oled.setCursor(0, 6);
					oled.print(F("LED graph."));
				}



				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { shift_rpm = shift_rpm - 10; }
					if (coloradjust1 == 64) { shift_rpm = shift_rpm + 10; }
					shift_rpm = constrain(shift_rpm, 0, 20000);

					if (prev_variable != shift_rpm) {
						oled.set1X();
						oled.setFont(utf8font10x16);
						oled.setCursor(35, 0);
						oled.print(F("SHIFT RPM"));
						oled.set2X();
						oled.setFont(lcdnums12x16);
						oled.setCursor(0, 2);
						if ((prev_variable >= 10000 && shift_rpm <10000) ||
							(prev_variable >= 1000 && shift_rpm <1000)) {
							oled.clearToEOL();
						}
						oled.print(shift_rpm);
						prev_variable = shift_rpm;
					}

					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						prev_variable = 0;
						exitmenu();
					}
				}
				break;


			case 5:  //SMOOTHING (conditioning)
				if (menu_enter == 0) {
					oled.setCursor(30, 0);
					oled.print(F("SMOOTHING"));
					oled.setCursor(0, 2);
					oled.print(F("Select ON if you get"));
					oled.setCursor(0, 4);
					oled.print(F("erratic or jumpy"));
					oled.setCursor(0, 6);
					oled.print(F("RPM values."));
					prev_variable = -1;
				}

				while (menu_enter == 1) {

					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { smoothing--; }
					if (coloradjust1 == 64) { smoothing++; }
					smoothing = constrain(smoothing, 0, 1);

					if (prev_variable != smoothing) {
						prev_variable = smoothing;
						if (smoothing) {
							oled.clear();
							oled.set1X();
							oled.setFont(utf8font10x16);
							oled.setCursor(30, 0);
							oled.print(F("SMOOTHING"));
							oled.set2X();
							oled.setFont(utf8font10x16);
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("ON"));
						}
						else {
							oled.clear();
							oled.set1X();
							oled.setFont(utf8font10x16);
							oled.setCursor(30, 0);
							oled.print(F("SMOOTHING"));
							oled.set2X();
							oled.setFont(utf8font10x16);
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("OFF"));
						}

					}
					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						prev_variable = 255;
						exitmenu();
					}
				}
				break;


			case 6:  // PULSES PER REVOLUTION
				if (menu_enter == 0) {
					oled.setCursor(0, 0);
					oled.print(F("PULSES PER ROTATION"));
					oled.setCursor(0, 2);
					oled.print(F("Select your engine size"));
					oled.setCursor(0, 4);
					oled.print(F("2.0 = 4cyl  3.0 = 6cyl"));
					oled.setCursor(0, 6);
					oled.print(F("4.0 = 8cyl"));
				}
				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { cal--; }
					if (coloradjust1 == 64) { cal++; }
					cal = constrain(cal, 1, 255);

					if (prev_cal != cal) {
						oled.set1X();
						oled.setFont(utf8font10x16);
						oled.setCursor(0, 0);
						oled.print(F("PULSES PER ROTATION"));
						oled.set2X();
						oled.setFont(lcdnums12x16);
						oled.setCursor(0, 2);
						oled.print((cal / 10));
						oled.setCursor(25, 2);
						oled.print(F("."));
						oled.setCursor(45, 2);
						oled.print((cal % 10));
						prev_variable = shift_rpm;
						prev_cal = cal;
					}

					rpm = long(60e7 / cal) / (float)interval;

					oled.set1X();
					oled.setFont(lcdnums12x16);
					oled.setCursor(0, 6);
					if ((display_rpm >= 10000 && rpm <10000) ||
						(display_rpm >= 1000 && rpm <1000)) {
						oled.clearToEOL();
					}
					oled.print(rpm);
					display_rpm = rpm;
					oled.set1X();
					oled.setFont(utf8font10x16);
					oled.setCursor(65, 6);
					oled.print(F("RPM"));

					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						prev_cal = 0;
						//                display.setColon(false);
						exitmenu();
					}
				}
				break;


			case 7:  // NUMBER OF LEDS

				if (menu_enter == 0) {
					oled.setCursor(20, 0);
					oled.print(F("NUMBER OF LEDS"));
					oled.setCursor(0, 2);
					oled.print(F("Set the number of LEDS"));
					oled.setCursor(0, 4);
					oled.print(F("in the light strip"));
					oled.setCursor(0, 6);
					oled.print(F(""));
				}


				while (menu_enter == 1) {

					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { NUMPIXELS--; }
					if (coloradjust1 == 64) { NUMPIXELS++; }
					NUMPIXELS = constrain(NUMPIXELS, 0, 32);

					if (prev_variable != NUMPIXELS) {
						if ((NUMPIXELS < 10) && prev_variable >= 10) {
							oled.set2X();
							oled.setFont(lcdnums12x16);
							oled.setCursor(0, 2);
							oled.clearToEOL();
						}

						prev_variable = NUMPIXELS;
						oled.set1X();
						oled.setFont(utf8font10x16);
						oled.setCursor(20, 0);
						oled.print(F("NUMBER OF LEDS"));
						oled.set2X();
						oled.setFont(lcdnums12x16);
						oled.setCursor(0, 2);
						oled.print(NUMPIXELS);

					}
					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						prev_variable = 0;
						exitmenu();

						oled.set1X();
						oled.setFont(utf8font10x16);
						oled.setCursor(0, 0);
						oled.clearToEOL();
						oled.print(F("REBOOT REQUIRED"));
						oled.set2X();
						oled.setFont(lcdnums12x16);
						oled.setCursor(0, 2);
						oled.clearToEOL();
						for (int x = 3; x>-1; x--) {
							oled.clearToEOL();
							oled.setCursor(55, 2);
							oled.print(x);
							delay(1000);
						}

						writeEEPROM();
						resetFunc();
					}
				}
				break;


			case 8:  // Color Segmentation   

				if (menu_enter == 0) {
					oled.setCursor(0, 0);
					oled.print(F("COLOR SEGMENTS"));
					oled.setCursor(0, 2);
					oled.setFont(utf8font10x16);
					oled.print(F("Sets the width of the"));
					oled.setCursor(0, 4);
					oled.print(F("colors across the LED"));
					oled.setCursor(0, 6);
					oled.print(F("strip."));
				}

				if (menu_enter == 1) {
					loadallcolors();
					build_segments();
					menu_enter = 2;
					current_seg_number = 1;
					seg_mover = 0;
					buildarrays();
					exitmenu();
				}
				break;


			case 9:  // PIXEL ANIMATION MODE

				if (menu_enter == 0) {
					oled.setCursor(0, 0);
					oled.print(F("ANIMATION MODE"));
					oled.setCursor(0, 2);
					oled.print(F("Choose the animation"));
					oled.setCursor(0, 4);
					oled.print(F("style for the LED strip"));
					oled.setCursor(0, 6);
					oled.print("");
					prev_animation = 0;
				}


				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { pixelanim--; }
					if (coloradjust1 == 64) { pixelanim++; }
					pixelanim = constrain(pixelanim, 1, 4);

					if (prev_animation != pixelanim) {
						if (DEBUG) { Serial.println("Animation Change"); }
						oled.setCursor(0, 0);
						oled.print(F("ANIMATION MODE"));

						prev_animation = pixelanim;
						loadallcolors();

						if (pixelanim == 1) {
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("LEFT TO RIGHT"));
							for (int a = 0; a<NUMPIXELS; a++) {
								strip.setPixelColor(a, color1);
								strip.show();
								delay(50);
							}

						}
						else if (pixelanim == 2) {
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("CENTER TO OUT"));
							for (int a = NUMPIXELS / 2; a<NUMPIXELS; a++) {
								strip.setPixelColor(a, color1);
								strip.setPixelColor(NUMPIXELS - a, color1);
								strip.show();
								delay(75);
							}

						}
						else if (pixelanim == 3) {
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("RIGHT TO LEFT"));
							for (int a = NUMPIXELS; a>-1; a--) {
								strip.setPixelColor(a, color1);
								strip.show();
								delay(50);
							}
						}
						else if (pixelanim == 4) {
							oled.setCursor(0, 2);
							oled.clearToEOL();
							oled.print(F("OUT TO CENTER"));
							for (int a = NUMPIXELS; a>(NUMPIXELS / 2) - 1; a--) {
								strip.setPixelColor(a, color1);
								strip.setPixelColor(NUMPIXELS - a, color1);
								strip.show();
								delay(75);
							}

						}
						oled.setCursor(0, 6);
						oled.clearToEOL();
						oled.print(F("PUSH TO SAVE"));
						strip.clear();
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						oled.clear();
						oled.setCursor(0, 2);
						oled.print(F("YOU MUST REDO"));
						oled.setCursor(0, 4);
						oled.print(F("SEGMENTS. PLEASE WAIT"));
						oled.print("");

						delay(2000);
						oled.clear();
						build_segments();
						loadallcolors();
						exitmenu();
					}
				}
				break;



			case 10: //Adjust Color #1 

				oled.setCursor(0, 0);
				oled.print(F("SET COLOR 1"));
				oled.setCursor(0, 2);
				oled.print(F("Set the color of the"));
				oled.setCursor(0, 4);
				oled.print(F("first LED segment."));
				oled.setCursor(0, 6);
				oled.print("");

				while (menu_enter == 1) {

					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { c1--; }
					if (coloradjust1 == 64) { c1++; }
					c1 = constrain(c1, 0, 255);

					if (prev_color != c1) {
						prev_color = c1;
						color1 = load_color(c1);
						testlights();
						//testlights(1);
					}

					if (digitalRead(button_pin) == LOW) {
						delay(250);
						prev_color = 0;
						menu_enter = 2;
						exitmenu();
					}
				}
				break;



			case 11: //Adjust Color #2 

				oled.setCursor(0, 0);
				oled.print(F("SET COLOR 2"));
				oled.setCursor(0, 2);
				oled.print(F("Set the color of the"));
				oled.setCursor(0, 4);
				oled.print(F("second LED segment."));
				oled.setCursor(0, 6);
				oled.print("");


				while (menu_enter == 1) {

					int coloradjust1 = rotary_process();

					if (coloradjust1 == -128) { c2--; }
					if (coloradjust1 == 64) { c2++; }
					c2 = constrain(c2, 0, 255);

					if (prev_color != c2) {
						prev_color = c2;
						color2 = load_color(c2);
						testlights();
						//testlights(2);
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						prev_color = 0;
						menu_enter = 2;
						exitmenu();
					}
				}
				break;

			case 12: //Adjust Color #3 

				oled.setCursor(0, 0);
				oled.print(F("SET COLOR 3"));
				oled.setCursor(0, 2);
				oled.print(F("Set the color of the"));
				oled.setCursor(0, 4);
				oled.print(F("third LED segment."));
				oled.setCursor(0, 6);
				oled.print("");

				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { c3--; }
					if (coloradjust1 == 64) { c3++; }
					c3 = constrain(c3, 0, 255);

					if (prev_color != c3) {
						prev_color = c3;
						color3 = load_color(c3);
						testlights();
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						prev_color = 0;
						menu_enter = 2;
						exitmenu();
					}
				}
				break;

			case 13: //Adjust Shift Color 

				oled.setCursor(0, 0);
				oled.print(F("SHIFT COLOR"));
				oled.setCursor(0, 2);
				oled.print(F("Set the color of the"));
				oled.setCursor(0, 4);
				oled.print(F("shift flash."));
				oled.setCursor(0, 6);
				oled.print(F(""));

				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { c4--; }
					if (coloradjust1 == 64) { c4++; }

					c4 = constrain(c4, 0, 255);

					flclr1 = load_color(c4);

					for (int i = 0; i<NUMPIXELS + 1; i++) {
						strip.setPixelColor(i, flclr1);
					}

					strip.show();

					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						exitmenu();
					}
				}
				break;


			case 14:   //DEBUG MODE

				oled.setCursor(0, 0);
				oled.print(F("DEBUG MODE"));
				oled.setCursor(0, 2);
				oled.print(F("Turn ON to enable"));
				oled.setCursor(0, 4);
				oled.print(F("serial output via USB."));
				oled.setCursor(0, 6);
				oled.print(F("57600 Baud"));


				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { DEBUG--; }
					if (coloradjust1 == 64) { DEBUG++; }
					DEBUG = constrain(DEBUG, 0, 1);

					if (DEBUG == 1) {
						oled.set1X();
						oled.setFont(chippernutserial);
						oled.setCursor(78, 0);
						oled.print("c");
					}
					else {
						oled.set1X();
						oled.setFont(chippernutserial);
						oled.setCursor(78, 0);
						oled.clearToEOL();
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						strip.clear();
						strip.show();
						exitmenu();
					}
				}
				break;


			case 15:   //RESET

				oled.setCursor(0, 0);
				oled.print(F("SYSTEM RESET"));
				oled.setCursor(0, 2);
				oled.print(F("Restores default settings."));


				while (menu_enter == 1) {
					int coloradjust1 = rotary_process();
					if (coloradjust1 == -128) { rst--; }
					if (coloradjust1 == 64) { rst++; }
					rst = constrain(rst, 0, 1);
					oled.setCursor(0, 4);
					oled.print(F("Are you sure?"));

					if (rst == 1) {
						oled.setCursor(0, 6);
						oled.print(F("YES  "));
					}
					else {
						oled.setCursor(0, 6);
						oled.print(F("NO  "));
					}


					if (digitalRead(button_pin) == LOW) {
						delay(250);
						menu_enter = 2;
						strip.clear();
						strip.show();
						exitmenu();
						if (rst == 1) {
							oled.clear();
							oled.set1X();
							oled.setFont(utf8font10x16);
							oled.setCursor(0, 4);
							oled.print(F("Deleting EEPROM memory block:"));

							for (int i = 0; i < 512; i++) {
								EEPROM.write(i, 0);
								oled.setCursor(0, 6);
								oled.print(i);
								delay(1);
							}
							resetFunc();

						}
					}
				}
				break;

			}
		}
	}
}





