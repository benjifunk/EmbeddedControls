/*

Benjamin Van Alstyne

Thomas Carton 
Ryan Morrison

LAB 2
Section 2
Side A

10/1/18
Lab_2.c

File description:
	This program is aa simple game with three modes. The user selects
	  a mode with the slideswitch and then presses the button to select 
	  a mode. Instructions and scoring ate detailed at the start of each 
	  game mode.

	10/7/18 - Added function Flash_BILED() to flash R/G at the end of each
				game mode. 
			- Consolidated initialization functions
	10/8/18 - Updated timer/counter functionality for more accurate counting
				=> 16-bit
				=> reset counter to fraction of ms (in ISR) to increase
				   accuracy of overflows/ sec
*/

#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>

/* Global Variables */
__sbit __at 0xA0	SS0; // P2.0
__sbit __at 0xA1	SS1; // P2.1
__sbit __at 0xA2	PB;  // P2.2

__sbit __at 0xB0	LED0; // P3.0
__sbit __at 0xB1	LED1; // ...
__sbit __at 0xB2	LED2;
__sbit __at 0xB3	LED3;
__sbit __at 0xB4	BILEDG;
__sbit __at 0xB5	BILEDR;

volatile unsigned int _count = 0;
const unsigned char DEBUG = 1;
/* Function Prototypes */

void	Init_Everything	(void);
void	Timer0_ISR		(void) __interrupt 1;
void 	delay			(long d); // d: delay time in milliseconds

void	Test_Mode		(void); 
void	Mode1			(void);
void 	Mode2			(void);
void	Mode3			(void);

void	Light_LED			(unsigned char num); // num:{0,1,2,3,4}
void	Flash_LED			(void);
void 	Flash_BILED			(void);
unsigned char Analog_Read	(unsigned char pin); // Pot. connected to pin 1 of Port 1
void	Draw_Bar			(unsigned char length);   


 
void	Timer0_ISR		(void) __interrupt 1
{
	TR0 = 0;
	_count++;
//	TMR0 = 0x1C00; // reset counter. 
	TR0 = 1;
}
/**********************************************************************
MAIN FUNCTION
**********************************************************************/

void main(void)
{
	Sys_Init();
	putchar(' ');
	Init_Everything();

	srand(12);
	while(1)
		Test_Mode();
}

/**********************************************************************
INITIALIZATION FUNCTIONS
**********************************************************************/
void Init_Everything(void)
{
	P1MDIN 	&= ~0x01; // xxxx xxx0
	P1MDOUT	&= ~0x01; 
	P1		|= 0x01;

	P2MDOUT &= ~0x07; // xxxx x000
	P2		|= 0x07;  

	P3MDOUT |= 0x3F; // xx11 1111
	P3		|= 0x3F; // MAKE SURE DEM OFF


	CKCON |= 0x08;	// Timer0 uses SYSCLK: Bit 3 set
	TMOD  &= 0xFC;	// clear 4 LSB's
	TMOD  |= 0x00;	// Timer0 mode 00: 13-bit
	TR0    = 0;		// Stop Timer0
	TMR0   = 0x00;	// Rese t the 13-bit timer to 0


	REF0CN  = 0x03; // set: internal reference voltage
	ADC1CF &= 0xFD; // set converter to gain of 1
	ADC1CF |= 0x01; //
	ADC1CN  = 0x80; // enable converter 


	IE |= 0x82;		// enable timrer0, global interrupts
}

/**********************************************************************
GAME MODE AND SELECTION FUNCTIONS
**********************************************************************/

void Test_Mode(void)
{
	//unsigned char i;
	printf("Select mode with slide switches. Then press the pusbutton to enter game mode.\r\n");
	while(PB)
	{									
		if		(!SS0 && SS1)	printf("Mode 1 Selected\r");
		else if (SS0 && !SS1)	printf("Mode 2 Selected\r");
		else if (!SS0 && !SS1)	printf("Mode 3 Selected\r");
		else 					printf("Mode 4 Selected\r");
	}
	
	putchar('/n');			
								 
	if		(!SS0 && SS1)	Mode1();
	else if (SS0 && !SS1)	Mode2();
	else if (!SS0 && !SS1)	Mode3();
	else
	{
		printf("Mode 4 does not do anything.\r\n");
		delay(250);
		while(!PB);
	}
}

// MODE 1 - QUICK MATCH ****************************************************

void Mode1(void)
{
	unsigned char 	n 			= 0, 
					value 		= 0, 
					penalty 	= 0, 
					this_penalty = 0, 
					incorrect 	= 0,
					i;
	unsigned int	tcount = 0;

	printf("MODE 1 Instructions:\r\n");
	printf("A number of LED's will light on the protoboard and\r\n");
	printf("stay lit for a half second. Turn the potentiometer\r\n");
	printf("to turn on LED's until the same pattern appears. Then,\r\n");
	printf("Press the pushbutton to stop the timer. Quickest time\r\n");
	printf("results in the best score.\r\n\n");
	printf("Press pushbutton to begin...\r\n");

	while(PB);

	P3 	   |= 0x3F; // turn all LEDs on port 3 off
	BILEDR 	= 0;	// turn on red bi-LED
	delay(1000);

	for(i = 0; i < 5; i++)
	{
//		printf("For loop M1");
		_count = 0;
		incorrect = 0;
		BILEDR = 1; // indicate start of turn

		n = (rand() % 4);
		Light_LED(n);
//		printf("%d LEDS should be LIT\r\n", n); // DEBUG
		delay(550); // LED is lit for appx .5s
		
		TR0  = 1;	
		TR0  = 1;
		while(PB) 
		{
			tcount = _count;	// record the current value of _count at this point in code
//			printf("%u\r", _count);
			value = Analog_Read(0)/63; // read the pot A/D value on pin 1 and map to value
			Light_LED(value); 
		}
//	    printf("\n%u\r", _count);
		tcount = tcount; // cycles need to be wasted for _count or tcount to not be 0
		TR0  = 0;
		TR0  = 0;
		Flash_BILED();
//		printf("%\nu\r", _count);

		if (value != n)	this_penalty += 10;
	
		this_penalty += tcount/1000; // (1 per 500ms) if _counts overflows every 5 ms
		penalty += this_penalty;	 // add to total score
		printf("Penalty points this round	: %d\r\n", this_penalty);
		printf("Penalty points for %d rounds: %d\r\n", i+1, penalty);
		this_penalty = 0; // need to reset this round's score!
	} // end 5 tries
	printf("Final Score : %d\r\n\n", penalty);
	Flash_BILED();
}

// GAME 2 - 2 PLAYER MATCHEMUP *********************************************

void Mode2(void)
{
	// display brief instrucitons. High score wins for this game
	unsigned char 	score = 0,
					this_score = 0,
					value = 0,
					presses = 0,
					i, k;
	unsigned int overflows[5] = {3000, 2700, 2440, 2220, 2000};
	unsigned int j;
	P3 	  |= 0x3F;
	BILEDR = 0;

	printf("MODE 2 Instructions:\r\n");
	printf("User 1: Set the amount of LEDs with the potentiometer. \r\n");
	printf("\tYou have 1 second.\r\n");
	printf("User 2: Push the pushbuttons as many times as the number\r\n");
	printf("\tof LEDs that are lit. \r\n");
	printf("\tA quicker reaction results in a better score.\r\n\n");
	printf("Press pushbutton to begin...\r\n");
	while(PB)
	delay(250);

	for(i = 0; i < 5; i++)
	{
		// Get input from user 1
		_count 		= 0; // make sure count is 0
		BILEDR 		= 1;
		this_score 	= 0;
		presses = 0;

		TR0 		= 1;
		while( _count < 5000)
		{
			value = Analog_Read(0) / 63; 
			Light_LED(value);
		}
		TR0 = 0;
		P3 |= 0x30; //BILED off
				
		// Count presses from user 2
		Flash_LED(); // let them know to press
		_count = 0;
		TR0 = 1;
		while ( _count < 2*overflows[i+1])
			if(!PB)
			{
				presses++;
				for(k = 0; k < 4; k++)
					for(j = 0; j < 55296; j++); // debounce ~< 15ms
				printf("presses: %d\r",presses);
			}
		TR0 = 0;
// 		printf("\rValue   :%u\r\nPresses :%u\r\n", value, presses);

		// Compare presses to user 1's set value
		if(value == presses) this_score += 10; // increase user score
		Flash_LED();
		
		// Check to see if the reaction was quick enough
		 P3 &= ( _count < overflows[i+1])
		 	? ~0x10  // green 
			: ~0x20; // rec

		score += this_score;
		printf("\r\nPoints this round: %d\r\n", this_score);
		printf("Points total	 : %d\r\n", score);

		delay(1000);
		P3 |= 0x30; // turn off BILED
		delay(250);
	}// END 5 tries
	printf("Final Score : %d\r\n\n", score);
	Flash_BILED();
}

// MODE 3 - BAR GRAPH FOLLOWER

void Mode3(void)
{
	unsigned int 	score = 0;
	unsigned char	this_score = 0,
					value = 0,
					potval = 0,
					i,
					input;
	P3 	  |= 0x3F;
	BILEDR = 0;
	
	printf("MODE 3 Instructions:\r\n");
	printf("P1: Turn the potentiometer until the bar graph \r\n");
	printf("\tin the terminal window matches the amount of LEDs lit.\r\n\n");
//	printf("Press pushbutton to begin...\r\n");
	printf("Enter a seed:");
	input = getchar();
	srand(input);
//	while(PB);

	for(i = 0; i < 5; i++)
	{

		if(DEBUG) printf("Round %d\r\n", i+0); 
		value = rand() % 5;
		Light_LED(value);
		delay(500);
		BILEDR = 1;

		_count = 0;
		if(DEBUG) printf("SET POT\r\n");
		TR0 = 1;
		TR0 = 1;
		while(_count < 10000)
			Draw_Bar(potval = Analog_Read(0));
		TR0 = 0;
		TR0 = 0;
	
		potval = potval >> 2; // 0-63
		Flash_LED();

		value = value << 4;
		if(value >= 64) value = 63;
 
		P3 &= potval < value
			? ~0x10			// biledg
			: ~0x20;		// biledr

//		printf("value %u. potval %u\r\n",value,potval);
//		getchar();
		if(abs(value - potval) >= 10) this_score = 0;
		else this_score = 10 - abs((value - potval))*2; //
		score += this_score;

		printf("\r\nPoints this round: %d\r\n", this_score);
		printf("Points total	 : %u\r\n\n", score);

		P3 |= 0x30;
	}
		printf("Final Score : %d\r\n\n", score);
		Flash_BILED();
}


/**********************************************************************
VARIOUS UTILITY FUNCTIONS
**********************************************************************/

// LIGHT ARRAY OF LEDs FROM NONE TO 4		
void Light_LED(unsigned char n)
{
	P3 |= 0x0F;
	//if 		(n == 0)	P3 |= 0x0F;
	     if (n == 1)	P3 &= ~0x01;
	else if	(n == 2)	P3 &= ~0x03;
	else if (n == 3)	P3 &= ~0x07;
	else if (n == 4)	P3 &= ~0x0F;
}

// FLASH LED ARRAY 
void Flash_LED(void)
{	
	unsigned char i;
	P3 |= 0x0F;
	for(i = 0; i < 3; i++) 
	{	
		P3 &= ~0x0F;	// turn LEDs on
		delay(100); // call delay function and wait 500 ms.
		P3 |= 0x0F; // turn LEDs off
		delay(100);
	}
}

// FLASH BILED 
void Flash_BILED(void)
{	
	unsigned char i;
	for(i = 0; i < 3; i++) 
	{	
		
		BILEDR = 1;
		BILEDG = 0;	// turn LEDs on
		delay(200); // call delay function and wait 500 ms.
		BILEDR = 0;
		BILEDG = 1;
		delay(200);
		P3 |= 0x30;
	}
}

// READ VOLTAGE AT PIN p AND COMPARE WITH VREF
unsigned char Analog_Read(unsigned char p)
{
	AMX1SL	= p;
	ADC1CN &= ~0x20;
	ADC1CN |= 0x10;

	while(!(ADC1CN & 0x20));
	return ADC1;
}

// DELAY FOR d MILLISECONDS
void delay(long d)
{
	unsigned char i;
	TMR0 = 0x4C00; 		// reset timer0 to 19456 _counts
	TR0 = 1;			// Enable Timer0
	while( _count < d*2) // wait for 'd' ms (2 ovf per ms)
	{
		for(i = 0; i < 100; i++); // waste some clock cycles?
	} 
	TR0 = 0;		   	// disable Timer0
	_count = 0;
}

// DRAW BAR TO TERMINAL OF LENGTH length
void Draw_Bar(unsigned char length) 
{
     char i; 
     length = length >> 2; 					 // 255 /(integer) 4 = 63
     for(i=0; i < length; i++) putchar('#');   // print number of '#'
	 length = 63 - length;                   // clear the rest of the line
     for(i=0; i<length; i++) putchar(' ');   // print (63 – length) space
     putchar('|');                           // print end mark '|' 
     putchar('\r');                          // return to beginning of the line 
} 