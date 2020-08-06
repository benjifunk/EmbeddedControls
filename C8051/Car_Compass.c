/*
Benjamin Van Alstyne

Thomas Carton
Ryan Morrison

Section 2
Side	A
11/04/2018

LAB IV - SOURCE CODE

Description:
This source code allows the car to move towards a desired heading set by the user (default west).
Several parameters can be changed by the user without having to recomple and flash the source code.
The desired heading, steering gain, drive gain, and reaction distance can all be set using the lcd 
display. The maximum speed is set using a 1k potentiometer connected to an analog input. The car's 
battery is monitored for low-voltage state on an-other analog channel, and the car emits a warning
if the voltage gets low. The light sensor on the ranger is read over i2c, and if a light is shone on
it the car will reverse at full speed until the light is removed.

*/
#include <c8051_SDCC.h>
//#include <stdlib.h>
#include <stdio.h>
#include <i2c.h>

#define DM_MIN_PW		2030 // >1.1 ms pulsewidth
#define DM_NEUT_PW		2765 // 1.5 ms pulsewidth
#define DM_MAX_PW		3500 // <1.9 ms pulsewidth

#define	SM_LEFT 		2315 // determined using lab 3-1
#define SM_RIGHT 		3395 // "..."
#define SM_CTR 			2905 //

#define DEBUG 			1 	 // 1 for debug functions


__sbit __at 0xB7 		ENABLE_SW; // switch to enable all drive functions
__sbit __at 0xB6		BUZZER;

volatile unsigned int 	_count = 0,
						DM_PW,
						SM_PW,
						_d_heading = 2700;

volatile unsigned char  _in_range = 60,
				 		ranger_kp	= 25,
				 		steering_kp	= 25;
						

__bit		_ranger_flag,
			_compass_flag,
			_print_flag,
			_adc_batt_flag,
			_b_flag,
			FORWARD;

void 		 Init_Everything	(void)					;
void 		 delay				(long d)				;
void 		 PCA0_ISR 			(void) __interrupt 9	; // set up for 20ms period
void		 menu				(void);
void 		 Read_Ranger 		(unsigned int *rv, unsigned char *lv);
unsigned int Read_Compass		(void)					;
void 		 Set_Speed		 	(unsigned int ran, float kp, unsigned int spd);
void 		 Set_Steering		(unsigned int kp, unsigned int heading);

unsigned char Analog_Read		(unsigned char pin);

/**************** MAIN ***********************************************************/
void main( ) 
{
	// init stuff
	unsigned char 	batt_voltg	= 0,
					brightness  = 0;

	unsigned int	dist 		= 0,
				 	heading 	= 0,
					max_speed	= 0;
				 	

	Init_Everything();

	printf("Set heading, heading, range, dpw, spw\r\n");
	while(1)
	{
		max_speed = Analog_Read(6); //should scale appx 0-700 (700 ~= 255 * 2.74)
	//	printf("xpd %d\r", max_speed);
		max_speed = max_speed*2.7;

		if(!ENABLE_SW) // if the enable switch is activated...
		{
			if(batt_voltg > 9) // only attempt to drive motors if enough batt voltg.
			{
				if( _compass_flag)
				{
					heading = Read_Compass();
					Set_Steering(steering_kp, heading);
					_compass_flag = 0;
				}
				if( _ranger_flag)	
					{
						_ranger_flag = 0;
						Read_Ranger(&dist, &brightness);
						if (brightness > 245)
						{
							FORWARD = 0;
							BUZZER  = _b_flag; // beep, beep, beep, ...
						}
						else 
						{				  
							FORWARD = 1;
							BUZZER  = 1;
						}
						Set_Speed(dist, ranger_kp, max_speed);
					}
				if( _print_flag) // print ~ every half second
				{
					_print_flag = 0;
					printf("%4u, %4d, %4u, %4u, %4u,\r\n",
					_d_heading, heading, dist, DM_PW, SM_PW);
					lcd_clear();
					lcd_print("Set heading:%d\n", _d_heading); 
					lcd_print("Heading    :%d\n", heading);
					lcd_print("Range      :%d", dist);
				}
			}
			if( _adc_batt_flag)	
			{
				_adc_batt_flag = 0;
				batt_voltg = Analog_Read(7)/17;	// map battery voltage from 0 to 15 
				//printf("Battery Voltage: %d", batt_voltg);
			}
		}
		else
		{
			lcd_clear();
			PCA0CP2 = 0xFFFF - DM_NEUT_PW;
			PCA0CP0 = 0xFFFF - SM_CTR;
			menu();//, &desired_heading);
		}
	}
}	

void menu(void)
{
	unsigned int choice = 0;
	printf("MENU - '#' TO SELECT\r\n");
	printf("0. Quit(must enable drive switch)\r\n");
	printf("1. Heading\r\n2. Steering gain\r\n");
	printf("3. Drive Gain\r\n4.Reaction Dist\r\nEnter Choice: \r\n");

	lcd_clear();
	lcd_print("MENU\n");
	lcd_print("#: quit    2: Skp\n");
	lcd_print("1: Heading 3: Dkp\n");
	lcd_print("choice: ");
	choice = kpd_input(1);
	lcd_clear();
	switch(choice)
	{

		case 0:
			return;
			break; 
		case 1:
			lcd_print("Heading : %d\n", _d_heading);
			lcd_print("Input: ");
			printf("Enter heading on kepad.\r\n");
			choice = kpd_input(1);
			if (choice) _d_heading = choice;
			break;
		case 2:
			lcd_print("Steering Gain: %d\n", steering_kp);
			lcd_print("Input: ");
			printf("Enter steering gain on kepad.\r\n");
			choice = kpd_input(1);
			if (choice) steering_kp = choice;
			break;
		case 3:
			lcd_print("Drive Gain: %d\n", ranger_kp);
			lcd_print("Input: ");
			printf("Enter drive gain on keypad.\r\n");
			choice = kpd_input(1);
			if (choice) ranger_kp = choice;
			break;
		case 4:
			lcd_print("Reaction Dist: %d\n", _in_range);
			lcd_print("Input (10-100): ");
			printf("Enter distance on keypad.");
			choice = kpd_input(1);
			if (choice) _in_range = choice;
			break;
		default:
			menu();
			break;
	}
}


void Read_Ranger(unsigned int *range, unsigned char *light)
{
	unsigned char 	Data[3],
					addr 	= 0xE0;

	i2c_read_data(addr, 1, Data, 3); // read 2 bytes, starting at reg 2
	*light	 = (unsigned char)Data[0];
	*range   = (unsigned int)((Data[1] << 8) | Data[2]);
	Data[0] = 0x51; // start ping and store range in cm.
	i2c_write_data(addr, 0, Data, 1); // write one byte of data to reg 0
}

void Set_Speed(unsigned int range, float rkp, unsigned int max_spd)
{
	unsigned int  dm_min, dm_max;

	dm_min = DM_MIN_PW + max_spd;
	dm_max = DM_MAX_PW - max_spd;

	if (FORWARD)
		if (range < _in_range)
		{
			 DM_PW = (unsigned int)(dm_max - rkp*( _in_range - range)); //////// edit as linear mapping 
			 if (DM_PW < DM_NEUT_PW) DM_PW = DM_NEUT_PW;
		}
		else DM_PW = (unsigned int)(dm_max); //////// figure this out
	else DM_PW = (unsigned int)(DM_NEUT_PW + (2*rkp*range - 5*dm_min)/10.0); // will stop when rkp*range == dm_min

	PCA0CP2 = 0xFFFF - DM_PW;
}

void Set_Steering(unsigned int skp, unsigned int heading) // locals must be first lines of code for some reason
{
	int				error;
	error			= _d_heading - heading;

	if (abs(error)>1800)
	{
		if (error>1800)
		{
			error=abs(error)-3600;
		}
		else
		{
			error=3600-abs(error);
		}
	}

//	printf("error: %d\r",error); 
//	if (error>1800) error -= 3600; 
//	if (error< -1800) error += 3600;

	if(FORWARD) SM_PW = (unsigned int)(SM_CTR + skp/25.0*(error));
	else 		SM_PW = (unsigned int)(SM_CTR - skp/25.0*(error));

	if(SM_PW < SM_LEFT)  SM_PW = SM_LEFT;
	if(SM_PW > SM_RIGHT) SM_PW = SM_RIGHT;

	PCA0CP0 = 0xFFFF - SM_PW;
}

unsigned int Read_Compass( void )
{
	unsigned char 	Data[2],
					addr 	= 0xC0; // Compass is @ 0xC0
	unsigned int 	heading	= 0;

	i2c_read_data(addr,2,Data,2); // read 2 bytes, starting at reg 2
	heading = (((unsigned int)Data[0] << 8) | Data[1]);

	return heading; 
}

void PCA0_ISR( void ) __interrupt 9
{
	_count++;
	if (CF)
	{
		CF = 0;
		PCA0 = 28672; // 0x7000.. for 20ms pulsewidth 

		if(!(_count %   2)) _compass_flag 	= 1;
		if(!(_count %   4)) _ranger_flag 	= 1;
		if(!(_count %  12))	_print_flag 	= 1;
		if(!(_count %  40))
		{
			_adc_batt_flag  = 1; 
			_b_flag = !_b_flag;
		} //toggle buzzer flg ery 1 sec.
		if(!(_count % 120)) _count			= 0;
	}
	PCA0CN &= 0x40;
}

// READ VOLTAGE AT PIN p AND COMPARE WITH VREF
unsigned char Analog_Read(unsigned char pin)
{
	AMX1SL	= pin; 
	printf("");
	ADC1CN &= ~0x20;
	ADC1CN |= 0x10;

	while(!(ADC1CN & 0x20));
	return ADC1;
}

void delay(long d) // this delay function is based off of PCA0 overflow time (20ms)
{
	unsigned char i;
	while( _count < d*50) // overflows occur every 20 ms, 20*50=1000ms
	{
		for(i = 0; i < 100; i++); // waste some clock cycles?
	} 
	_count = 0;
}

void Init_Everything( void )
{
	Sys_Init();
	putchar(' ');

	P1MDOUT |= 0x04; // SET UP PORT I/O !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	P1MDOUT &= ~0xC0;
	P1MDIN  &= ~0xC0;
	P1		|=	0xC0;

	XBR0	 = 0x27;

	P3MDOUT &= ~0x80;
	P3MDOUT |=  0x40;
	P3 		|=  0xC0;
	

	PCA0MD 	 = 0x81; //configure PCA MODE to count @ SYSCLK/12
 	PCA0CPM2 =       // init module !! Needs to be set for each PCA module in use!!
	PCA0CPM0 = 
	PCA0CPM3 = 0xC2;
	PCA0CN 	 = 0x40; // Enable PCA counter
	
	EIE1	|= 0x08;	// Enable PCA0 interrupt
	EA		 = 1;		// Enable Global interrupts
		
	SMB0CR 	 = 0x93;
	SMB0CN 	|= 0x40;
	
	REF0CN  = 0x03; // set: internal reference voltage
//	ADC1CF &= 0xFD; // set converter to gain of 1
	ADC1CF |= 0x01; //
	ADC1CN  = 0x80; // enable converter 


	PCA0CP2 = 0xFFFF - DM_NEUT_PW;
	delay(1); // initialize the drive motor controller
}
