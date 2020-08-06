/*
Benjamin Van Alstyne

Thomas Carton
Ryan Morrison

Section 2
Side	A
11/17/2018

LAB V - SOURCE CODE

Description:



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
__sbit __at 0xB6		BILEDR;
__sbit __at 0xB5		BILEDG;

volatile unsigned int 	_count = 0,
DM_PW,
SM_PW;

volatile signed long
_x_error,
_y_error;


volatile unsigned char  _kdx	= 255,
						_kdy	= 100,
						_ki		= 10,
				 		_skp	= 100;
						
__bit		_accel_read_flag = 0,
			_print_flag = 0,
			_adc_batt_flag = 0,
			_b_flag = 0,
			FORWARD = 1;

volatile signed int 	_gx = 0,
				_gy = 0,
				_prev_gx = 0,
				_prev_gy = 0,
				_error_sum;

void 		 Init_Everything	(void)					;
void 		 delay				(long d)				;
void 		 PCA0_ISR 			(void) __interrupt 9	; // set up for 20ms period
void		 menu				(void);
void 		 Read_Accel			(void)					;
void 		 Set_Speed		 	(unsigned int spd);
void 		 Set_Steering		(void);

unsigned char Analog_Read		(unsigned char pin);

/**************** MAIN ***********************************************************/
void main( ) 
{
	// init stuff
	unsigned char 	batt_voltg  = 0, i, input;
	unsigned int	max_speed	= 0, dir = 0;
	unsigned char   counts = 0;

	Init_Everything();
	printf("g");
	lcd_print("g");

	Accel_Init_C();
	
	max_speed = 25; //should scale appx 0-700 (700 ~= 255 * 2.74)

	BILEDR = BILEDG = 1;
	lcd_clear();
	lcd_print("Place flat. Then press # to calib.\n");
	kpd_input(1);
	lcd_print("calibrating...");
	
	for (i = 0; i < 63; i++)
	{
		Read_Accel();
		_x_error += _gx;
		_y_error += _gy;
	}
	_x_error /= 64;
	_y_error /= 64;
	
	menu();

	lcd_clear();
	FORWARD = 0;
	printf("Input reverse direction:");
	input = kpd_input(1);
	while(!dir)
	{
		if(input == 7)
			dir = SM_RIGHT;
		else if(input == 9)
			dir = SM_LEFT;
		else input = kpd_input(1);
	}

	BILEDR = 0;
	PCA0CP0 = 0xFFFF - dir;

	_gx = _gy = 100;
	while (counts < 50)
	{
		if ( _gy < -27 ) counts++;
		else counts = 0;
		if ( _accel_read_flag)
		{
			Read_Accel();
			_gx -= _x_error;
			_gy -= _y_error;
			Set_Speed(max_speed);
		}
	}
	BILEDG = 0;
	BILEDR = 1;

	if (input == 7) PCA0CP0 = 0xFFFF - SM_LEFT;
	else PCA0CP0 = 0xFFFF - SM_RIGHT;
	FORWARD = 1;
	_kdx = 50;
	while(1)
	{
		_kdy = 2.77*Analog_Read(6);
		if(!ENABLE_SW) // if the enable switch is activated...
		{
			if(batt_voltg > 9) // only attempt to drive motors if enough batt voltg.
			{
				if( _accel_read_flag)
				{
					Read_Accel();
					_gx -= _x_error;
					_gy -= _y_error;

					//if(abs(_gx - _prev_gx) > 80) _gx = _prev_gx; // throw away extraneous values
					//if(abs(_gy - _prev_gy) > 80) _gy = _prev_gy;

					if (DM_PW < DM_NEUT_PW)
					{
					BILEDR = 0; 
					BILEDG = 1;
					} 
					if (DM_PW > DM_NEUT_PW) 
					{
						BILEDG = 0; 
						BILEDR = 1;
					}
					if (DM_PW > DM_NEUT_PW - 100 && DM_PW < DM_NEUT_PW + 100)
					{
						BILEDR = BILEDG = 1;
					}
					Set_Speed(max_speed);
					Set_Steering();

					_prev_gx = _gx;
					_prev_gy = _gy;

					
				}
				if( _print_flag) // print ~ every half second
				{
					_print_flag = 0;
					lcd_clear();
					lcd_print("Accel X    :%d\n", _gx);
					lcd_print("Accel Y    :%d\n", _gy);
					lcd_print("G XYS: %d %d %d\n", _kdx, _kdy, _skp);
					lcd_print("MPW: %d SPW %d\n", DM_PW, SM_PW);
					printf("%d, %d, %d, %d\r\n", _gx, _gy, DM_PW, SM_PW);
				}
			}
			if( _adc_batt_flag)	
			{
				_adc_batt_flag = 0;
				batt_voltg = Analog_Read(7)/17;	// map battery voltage from 0 to 15 
			//	printf("Battery Voltage: %d", batt_voltg);
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
	lcd_print("#: quit    2: kdx\n");
	lcd_print("1: kdy     3: skp\n");
	lcd_print("4: ki (drive)\n");
	choice = kpd_input(1);
	lcd_clear();

	switch(choice)
	{
		case 0:
			return;
			break; 
		case 1:
			lcd_print("input kdy: ");
			_kdy = kpd_input(1);
			break;
		case 2:
			lcd_print("input kdx: ");
			_kdx = kpd_input(1);
			break;
		case 3:
			lcd_print("input skp: ");
			_skp = kpd_input(1);
			break;
		case 4:
			lcd_print("input ki: ");
			_ki = kpd_input(1);
			break;
		default:
			menu();
			break;
	}
}

void Read_Accel(void)
{
	
	unsigned char i = 0, daaata[4];
	int gx = 0, gy = 0;
	daaata[0] = 0;
	_accel_read_flag = 0;

	gx = gy = 0;
	while (i < 8)
	{
		i++;
		while ((daaata[0] & 0x03) != 0x03)
		{
			i2c_read_data(0x3A, 0x27, daaata, 1);
		}
		i2c_read_data(0x3A, 0x28|0x80, daaata, 4);
		
		gx += (int)((daaata[1] << 8 | daaata[0]) >> 4);
		gy += (int)((daaata[3] << 8 | daaata[2]) >> 4);
	}	

	_gx = (gx >> 3);
	_gy = (gy >> 3);
}


void Set_Speed(unsigned int max_spd)
{
	unsigned int  dm_min, dm_max;

	//if(!FORWARD)
	//{
//		_gy = -_gy;
//	}

	dm_min = DM_MIN_PW + max_spd;
	dm_max = DM_MAX_PW - max_spd;
	
	DM_PW	  = (unsigned int)(DM_NEUT_PW - (signed long)_kdy*_gy/51.0);// set to go down ramp
	if (FORWARD) DM_PW	  += (unsigned int)((signed long)_kdx*abs(_gx)/51.0); // set to go foreward w/ tilt
	else DM_PW	  -= (unsigned int)((signed long)_kdx*abs(_gx)/51.0);
//	_error_sum += (signed int)( _gy + abs( _gx));

//	printf("  Error_: %d\r", _error_sum);

	if (DM_PW > dm_max) DM_PW = dm_max;
	if (DM_PW < dm_min) DM_PW = dm_min;

	PCA0CP2 = 0xFFFF - DM_PW;
}

void Set_Steering(void)
{
	
	if(FORWARD) SM_PW = (unsigned int)(SM_CTR + ( (signed long)_skp/51.0 * _gx));
	else SM_PW = (unsigned int)(SM_CTR + ((signed long)_skp/51.0 * _gx));

	if(SM_PW < SM_LEFT)  SM_PW = SM_LEFT;
	if(SM_PW > SM_RIGHT) SM_PW = SM_RIGHT;

	PCA0CP0 = 0xFFFF - SM_PW;
}

void PCA0_ISR( void ) __interrupt 9
{
	_count++;
	if (CF)
	{
		CF = 0;
		PCA0 = 28672; // 0x7000.. for 20ms pulsewidth 

		if (!(_count % 2)) _accel_read_flag = 1;
		if(!(_count %  12))	_print_flag 	= 1;
		if(!(_count %  48)) _adc_batt_flag  = 1; 
		if(!(_count % 128)) _count			= 0;
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

//	Accel_Init_C();

	P1MDOUT |=  0x04; // SET UP PORT I/O !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	P1MDOUT &= ~0xC0;
	P1MDIN  &= ~0xC0;
	P1		|=	0xC0;

	XBR0	 = 0x27;

	P3MDOUT &= ~0x80;
	P3MDOUT |=  0x60;
	P3 		|=  0xF0;
	

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
