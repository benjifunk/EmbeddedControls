/*
Benjamin Van Alstyne

Thomas Carton
Ryan Morrison

Section 2
Side	A
11/04/2018

LAB IV - SOURCE CODE

Description:
This source code is a modification of the code written for lab 4, adapted for use with the gondola. This code uses input from the compass in a closed-loop PD control algorithm to set the gondola to a desired heading. The derivative term is used to reduce overshoot that would otherwise affect the performance of the blimp. Ancillary procedures are performed as well, changing the thrust fan angles and their speed. A procedure for setting the neutral thrust fan angle is also given so that the user can calibrate the gondola before the main loop is entered.

*/
#include <c8051_SDCC.h>
//#include <stdlib.h>
#include <stdio.h>
#include <i2c.h>

#define TAIL_MIN_PW		2028 // >1.1 ms pulsewidth
#define TAIL_NEUT_PW	2765 // 1.5 ms pulsewidth
#define TAIL_MAX_PW		3502 // <1.9 ms pulsewidth

#define	THRUST_LOW_PW	2400 //
#define THRUST_HI_PW	3200 // "..."
#define THRUST_MID_PW	2905 //

#define DEBUG 			0 	 // 1 for debug functions

__sbit __at 0xB7 		ENABLE_SW; // switch to enable all drive functions
__sbit __at 0xB6		BUZZER;

volatile unsigned int 	_count = 0,
_tail_fan_pw,
_past_headings[4] = { 0 },
_thrust_pw_neut,
_thrust_angle_pw,
_avg_heading,
_dir;

volatile signed int _previous_error = 0;

volatile unsigned char  _in_range = 60,
_heading_kD = 10,
_heading_kP = 100;

__bit		_ranger_flag = 0, // boolean flags
_compass_flag = 0,
_print_flag = 0,
_adc_batt_flag = 0,
_turn_flag = 0;

/*************** FUNCTION DECLARATIONS ***********************************/

void 		 Init_Everything(void);
void 		 delay(long d);
void 		 PCA0_ISR(void) __interrupt 9; // set up for 20ms period
unsigned int Read_Ranger(void);		// return ping from ranger over SMB
unsigned int Read_Compass(void);	// return the current heading from compass over SMB

void 		 Set_Thrust_Angle(void);		// called after thrust angle initialization, by ranger
void 		 Set_Tail_Fan(unsigned int angle); // called in loop, sets tail fan speed
void		 Set_Thrust(void);				// thrust fan speed controlled by ranger
unsigned char Analog_Read(unsigned char pin);

void Set_Init_Thrust_Angle(void); // called beginning of main

/**************** MAIN ***********************************************************/
void main()
{
	// init stuff
	unsigned char 	batt_voltg = 0,
		i = 0;

	unsigned int
		heading = 0,
		deflection = 0;

	__bit			cw = 0;

	Init_Everything();
	printf("g");
	lcd_print("g");

	lcd_clear();
	lcd_print("Initialized\n");

	// CALL ROUTINE TO SET NEUTRAL THRUST ANGLE (CHANGEd #DEFINE to u_int)
	Set_Init_Thrust_Angle();

	// RANGER LOOP - WHILE (!LOCKed) 
	Set_Thrust_Angle();
	printf("thrust angle locked\r\n");

	Set_Thrust();
	printf("thrust locked\r\n");

	// ENTER PARAMETERS FOR FAN
	lcd_clear();
	printf("Enter desired heading on keypad");
	heading = kpd_input(1); // the centerpoint around which the gondola will oscillate
	lcd_clear();

	printf("Enter deflection angle on keypad:");
	deflection = kpd_input(1); // the degree of oscillation desired (note a longer delay is needed > 45*)
	lcd_clear();

	// ONCE THRUST SERVO IS LOCKED, ENTER OSCILLATING FAN LOOP
	while (1)
	{
		if (_turn_flag) // turn flag set in PCA0_ISR, apx every 3 sec, may increase
		{
			_turn_flag = 0;
			cw = !cw; // !cw = ccw
		}
		if (cw)	Set_Tail_Fan(heading + deflection); // if set clockwise turn to right of setpoint
		else	Set_Tail_Fan(heading - deflection); // otherwise, turn to left of setpoint

		// v - used to tune P,D gains
		//Set_Tail_Fan(heading);
		// ^ 

		if (_print_flag)
		{
			lcd_clear();						// print the direction we should be turning 
			if (!cw) lcd_print("Clockwise\n");
			else lcd_print("Anti-clockwise\n");

			lcd_print("Head: %d  Def: %d\n", heading, deflection); // print desired heading and deflect.
			_print_flag = 0;
			printf("%d, %d, %d, %d, %d, %d\r\n", heading, _dir, cw, _avg_heading, _tail_fan_pw, _previous_error); // printf other necessary data over the wireless UART interface
		}
	}
}

/* Function  : READ_RANGER 
* Parameters : None
* Returns    : unsigned int
* Description: use the SMB to interface with the ranger and return a value corresponding to the distance from the (top) of the gondola, in centimeters
*/
unsigned int Read_Ranger(void)
{
	unsigned char 	Data[2];
	unsigned int range;

	i2c_read_data(0xE0, 2, Data, 2); // read 2 bytes, starting at reg 2
	range = (unsigned int)((Data[1] << 8) | Data[0]);
	Data[0] = 0x51; // start ping and store range in cm.
	i2c_write_data(0xE0, 0, Data, 1); // write one byte of data to reg 0

	return range;
}

/* Function  : READ_COMPASS
* Parameters : None
* Returns    : unsigned int
* Description: use the SMB to interface with the compass and return a value corresponding to a tenth of a degree azimuth (measured CW) from calibrated north.
*/
unsigned int Read_Compass(void)
{
	unsigned char 	Data[2]; // Compass is @ 0xC0
	unsigned int 	heading = 0;

	i2c_read_data(0xC0, 2, Data, 2); // read 2 bytes, starting at reg 2, store in buffer Data
	heading = (((unsigned int)Data[0] << 8) | Data[1]); // shift high byte over and combine with low byte

	return heading; // return the 16-bit value
}

/* Function  : READ_RANGER
* Parameters : unsigned int
* Returns    : none
* Description: Do necessary calculations and output a PWM signal to thrust motor controller(s) to change fan speeds. Note, parts of code that perform extra averaging and use the side thrust motors have been commented out. I wanted to keep these in case we wanted to use them.
*/
void Set_Tail_Fan(unsigned int desired_angle)
{
	//unsigned int avg_heading = 0; 
	signed int error = 0;
	signed long tmp_pw, neg_tmp_pw; // for calculations - neg_tmp_pw used for opposite-spinning fans
	// unsigned char i;
	// unsigned int dinkle;

	if (_compass_flag)
	{
		_compass_flag = 0; // reset compass flag!
		/*
				for (i = 0; i < 3; i++) // obviously, first 80ms will be garbage (slow response)
				{
					_past_headings[i + 1] = _past_headings[i];// move readings down in list
				}
				_past_headings[0] = Read_Compass(); // place new compass reading at top of list


				for (i = 0; i < 3; i++)
				{
					_avg_heading = (unsigned int)(_past_headings[i] + _past_headings[i + 1]) / 2.0; // average the list values
				}
				error = desired_angle - _avg_heading;
		*/
		_dir = Read_Compass(); 
		error = desired_angle - _dir; // error is the set point minus the current direction

		if (abs(error) > 1800) // must limit error to 0-180 degrees
		{
			if (error > 1800) // a bit of unneccessary testing -- if error is positive...
			{
				error = abs(error) - 3600; // make negative 0 - 180
			}
			else
			{
				error = 3600 - abs(error); // make positive 0 - 180
			}
		}

		tmp_pw = (signed long)TAIL_NEUT_PW + (signed long)_heading_kP * (signed long)error + (signed long)_heading_kD * (signed long)(error - _previous_error); // typecasting as in worksheet 11
		
		//	neg_tmp_pw = (signed long)TAIL_NEUT_PW - (signed long)_heading_kP * (signed long)error - (signed long)_heading_kD * (signed long)(error - _previous_error);
		// ^ for reverse fans

		// limit pulsewidth to minimum and maximum speeds
		if (tmp_pw > (signed long)TAIL_MAX_PW) tmp_pw = (signed long)TAIL_MAX_PW;
		else if (tmp_pw < (signed long)TAIL_MIN_PW) tmp_pw = (signed long)TAIL_MIN_PW;

		_tail_fan_pw = (unsigned int)tmp_pw;
		//	dinkle = (unsigned int)neg_tmp_pw;

		// SET PCA CAPTURE COMPARE MODULES TO SET CURRENT PW	
		_previous_error = error;
		PCA0CP0 = 0xFFFF - _tail_fan_pw; // CCM0 controls rudder fan
	//	PCA0CP2 = 0xFFFF - _tail_fan_pw;
	//	PCA0CP3 = 0XFFFF - dinkle;

	}

}

/* Function  : Set_Thrust
* Parameters : None
* Returns    : None
* Description: Do calculations and set thrust fan speed.
*/
void Set_Thrust(void)
{
	__bit			thrust_lock = 0; // flag to indicate whether consistent readings are made

	unsigned int	range = 0,
		prev_range = 0,
		thrust_pw = TAIL_NEUT_PW;

	unsigned char	timeout = 0;

	while (!thrust_lock)
	{
		if (_ranger_flag)
		{
			_ranger_flag = 0;
			range = (unsigned int)Read_Ranger(); // get current distance reading
			thrust_pw = THRUST_MID_PW + (unsigned int)((range - 50) * .10); //gain = 10, nom height = 40cm
			if (thrust_pw < TAIL_MIN_PW) thrust_pw = TAIL_MIN_PW; // limit the pw values
			if (thrust_pw > TAIL_MAX_PW) thrust_pw = TAIL_MAX_PW;
			PCA0CP2 = PCA0CP3 = 0xFFFF - thrust_pw; // set CPM's on left and right fans

			if (abs(prev_range - range) < 8) timeout++; // check if range is within a certain value
			else timeout = 0; // reset counter if range is changing

			if (timeout > 30) thrust_lock = 1; // set thrust lock and break loop
			prev_range = range;

			if (_print_flag)
			{
				_print_flag = 0;
				printf("Range: %d\r", range); // used to determine code is working properly
			}
		}
	}
}

/* Function  : SET_THRUST_ANGLE
* Parameters : None
* Returns    : None
* Description: Set the thrust angle of the side fans using the value returned by the ranger.
*/
void Set_Thrust_Angle(void)
{
	__bit			thrust_lock = 0;

	unsigned int	range = 0,
		prev_range = 0,
		thrust_angle_pw = THRUST_MID_PW; // initialize to 'center' value (different on each gondola)

	unsigned char	timeout = 0;

	while (!thrust_lock)
	{
		if (_ranger_flag)
		{
			_ranger_flag = 0;
			range = Read_Ranger(); // get range
			thrust_angle_pw = THRUST_MID_PW + (unsigned int)((range - 100) * .10); //gain = 10, nom height = 40cm
			if (_thrust_angle_pw < THRUST_LOW_PW) _thrust_angle_pw = THRUST_LOW_PW; // limit pw values
			if (_thrust_angle_pw > THRUST_HI_PW) _thrust_angle_pw = THRUST_HI_PW;

			PCA0CP1 = 0xFFFF - thrust_angle_pw; // set CPM1 to the desired pw value

			if (abs(prev_range - range) < 7) timeout++; // check to see if consistent ranges are made
			else timeout = 0;

			if (timeout > 30) thrust_lock = 1; // enable escape loop
			prev_range = range;
			printf("Range: %d\r", range);
		}
	}
}

/* Function  : SET_INIT_THRUST_ANGLE
* Parameters : None
* Returns    : None
* Description: Used to calibrate the thrust fan angle. Done through keyboard input over terminal. This would be needed to allow the blimp to hover without drifting forward or backward.
*/
void Set_Init_Thrust_Angle(void)
{
	unsigned int thrust_angle_pw = THRUST_MID_PW; // initial mid pw different on each blimp
	unsigned char input;
	PCA0CP1 = 0xFFFF - thrust_angle_pw; // set the initial thrust angle

	printf("Enter u: up d: down q: save and quit... CTR_PW: %d\r", thrust_angle_pw);
	input = getchar();
	while (input != 'q')
	{
		if (input == 'u' || input == 'U')
			thrust_angle_pw += 10;
		else if (input == 'd' || input == 'D')
			thrust_angle_pw -= 10;

		//if (thrust_angle_pw < THRUST_LOW_PW) thrust_angle_pw = THRUST_LOW_PW;
		//else if (thrust_angle_pw > THRUST_HI_PW) thrust_angle_pw = THRUST_HI_PW;

		PCA0CP1 = 0xFFFF - thrust_angle_pw; 

		input = getchar();
		printf("\r Pulsewidth: %d", thrust_angle_pw);
	}
	_thrust_pw_neut = thrust_angle_pw; // after loop is broken, set the neutral position to the current pw
}

/* Function  : PCA0_ISR
* Parameters : None
* Returns    : None
* Description: ISR to increment _count every 20ms, which toggles flags.
*/
void PCA0_ISR(void) __interrupt 9
{
	_count++;

	if (CF)
	{
		CF = 0;
		PCA0 = 28672; // 0x7000.. for 20ms pulsewidth 

		if (!(_count % 2)) _compass_flag++; // 40ms flag
		if (!(_count % 4)) _ranger_flag++; // 80ms flag
		if (!(_count % 10))	_print_flag++; // 200ms
		if (!(_count % 40))	_adc_batt_flag++; // 780 ms 
		if (!(_count % 200)) // 4s flag: 20 prints, 50 ranges, 100 compass readings, 5 battery readings
		{
			_turn_flag++;
			_count = 0;
		}
	}
	PCA0CN &= 0x40;
}

/* Function  : ANALOG_READ
* Parameters : Unsigned char
* Returns    : unsigned char
* Description: Read analog voltage on pin and return value 0-255
*/
// READ VOLTAGE AT PIN p AND COMPARE WITH VREF
unsigned char Analog_Read(unsigned char pin)
{
	AMX1SL = pin; // select pin
	printf(""); // wait for a tickle
	ADC1CN &= ~0x20; 
	ADC1CN |= 0x10;
	// start conversion, wait for conversion to complete
	while (!(ADC1CN & 0x20));
	return ADC1;
}

void delay(long d) // this delay function is based off of PCA0 overflow time (20ms)
{
	unsigned char i;
	while (_count < d * 50) // overflows occur every 20 ms, 20*50=1000ms
	{
		for (i = 0; i < 100; i++); // waste some clock cycles?
	}
	_count = 0;
}

void Init_Everything(void)
{
	Sys_Init();
	putchar(' ');

	P1MDOUT |= 0x04; // SET UP PORT I/O !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	P1MDOUT &= ~0xC0;
	P1MDIN &= ~0xC0;
	P1 |= 0xC0;

	XBR0 = 0x25;

	P3MDOUT &= ~0x80;
	P3MDOUT |= 0x40;
	P3 |= 0xC0;


	PCA0MD = 0x81; //configure PCA MODE to count @ SYSCLK/12
	PCA0CPM2 =       // init module !! Needs to be set for each PCA module in use!!
		PCA0CPM0 =
		PCA0CPM1 =
		PCA0CPM3 = 0xC2;
	PCA0CN = 0x40; // Enable PCA counter

	EIE1 |= 0x08;	// Enable PCA0 interrupt
	EA = 1;		// Enable Global interrupts

	SMB0CR = 0x93;
	SMB0CN |= 0x40;

	REF0CN = 0x03; // set: internal reference voltage
//	ADC1CF &= 0xFD; // set converter to gain of 1
	ADC1CF |= 0x01; //
	ADC1CN = 0x80; // enable converter 

	PCA0CP0 = 0xFFFF - TAIL_NEUT_PW;
	PCA0CP2 = 0xFFFF - TAIL_NEUT_PW;
	PCA0CP3 = 0xFFFF - TAIL_NEUT_PW;
	delay(1); // initialize the drive motor controller
}
