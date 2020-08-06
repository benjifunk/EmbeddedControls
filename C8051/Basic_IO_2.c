/*
Comments


*/
#include <c8051_SDCC.h>
//#include <stdlib.h>
#include <stdio.h>
#include <i2c.h>

#define LED_MIN_PW		0 // 
#define LED_MAX_PW		65535 // 

//#define DEBUG 		1 	 // 1 for debug functions

__sbit __at 0xB5 LED_Switch;

volatile unsigned int 	_count = 0,
						_delay = 0,
						_ranger_flag = 0,
						_print_ctr = 0,
						LED_PW;


void Init_Everything	(void)					;
void delay				(long d)				;
void PCA0_ISR 			(void) __interrupt 9	; // set up for 20ms period
void Set_LED(unsigned int);

void 			ADC_Init		(void);
unsigned char 	read_AD_input	(unsigned char pin_number);

void main( ) 
{
	unsigned int lum = 0;
	Init_Everything();
	
	printf("\r\nInitialized.\r\n");

	while(1)
	{
//	
		printf("Enter pulsewidth: ");
		lum = getchar();
		printf("Enter frequency: ");
		freq = getchar();

		if( _print_ctr > 6) // print ~ every half second
		{
			printf("Light detected: %d     ", lum);
			printf("LED pulsewidth: %ul     \r", LED_PW);
		}
	}
}	


void Set_LED(unsigned int br)
{
	LED_PW = 256*br;

	if(LED_PW < LED_MIN_PW) LED_PW = LED_MIN_PW;
	if(LED_PW > LED_MAX_PW) LED_PW = LED_MAX_PW;
	
	if(br > 215) LED_PW = LED_MAX_PW;
	PCA0CPL3 = 0xFFFF - LED_PW;
	PCA0CPH3 = (0xFFFF - LED_PW) >> 8;
	
//	printf("Pulsewidth : %ul\r", LED_PW);
	
}

void PCA0_ISR( void ) __interrupt 9
{
	_count++;
	_delay++;
	if (CF)
	{
		CF = 0;
		PCA0 = 28672; // 0x7000.. for 20ms pulsewidth 

		if( _count > 4) //20ms*4 = 80ms 
		{
			_ranger_flag = 1;
			_print_ctr++;
			_count		 = 0;
		
		}
	}
	PCA0CN &= 0x40;
}

void delay(long d) // this delay function is based off of PCA0 overflow time (20ms)
{
	unsigned char i;
	while( _delay < d*50) // overflows occur every 20 ms, 20*50=1000ms
	{
		for(i = 0; i < 100; i++); // waste some clock cycles?
	} 
	_delay = 0;
}

void Init_Everything( void )
{
	Sys_Init();
	putchar(' ');

	P3MDOUT &= ~0xE0; 
	P3		|= 0xE0; // 111X XXXX
	
	P1MDIN 	&= ~0x80;	// P1.6 to recieve analog input x1xx xxxx
	P1MDOUT	&= ~0x80;	// P1.6 is input
	P1		|=  0x80;	// P1.6 is high impedance

	XBR0	 = 0x27;

	PCA0MD 	 = 0x81; //configure PCA MODE : to count @ SYSCLK/12
 	PCA0CPM2 = 0xC2;     // init module 0 !! Needs to be set for each PCA module !!
	PCA0CPM0 = 
	PCA0CPM3 = 0xC2;
	PCA0CN 	 = 0x40; // Enable PCA counter !!!
	
	EIE1	|= 0x08;	// Enable PCA0 interrupt
	EA		 = 1;		// Enable Global interrupts
		
	SMB0CR 	 = 0x93;
	SMB0CN 	|= 0x40;

	REF0CN  = 0x03; // use internal reference voltage
	ADC1CF &= ~0x03; // set converter to gain of 0.5... bits 10 := 00
	ADC1CN  = 0x80; // enable converter
	Interrupt_Init(); 
	
}


unsigned char read_AD_input(unsigned char p)
{
	AMX1SL	= p;		// set which pin signal is routed to
	ADC1CN &= ~0x20;	// clear ADC1 Conversion Complete Flag
	ADC1CN |= 0x10;		// set AD1BUSY... will trigger AD1INT on
						//	falling edge...
	while(!(ADC1CN & 0x20)); // wait for AD1BUSY to be triggered
	return ADC1;		// return A/D value [0:255]
}

