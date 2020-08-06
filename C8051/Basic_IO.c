/*
	Benjamin Van Alstyne
	Section 02
	Side A
	09/12/2018

	File name: Lab1_1.c
	Program Description:
	
	This program demonstrates the use of inputs and outputs on the c8051_020
	evaluation board. Using two pushbuttons, slide switch, buzzer, led and 
	bi-coled; the program will perform the following operations:
	(1) When the SPDT switch is 'off', the LED is on (buttons not pushed)
	(2) When SPDT switch is 'on', LED is off.
	(3) When SPDT switch is 'on' and pushbutton 1 pushed, bi-LED is green.
	(4) When SPDT switch is 'on' and pushbutton 2 pushed, bi-LED is red. 
	(5) When SPDT switch is 'on' and both pushbuttons pushed, buzzer is on.
*/

#include <c8051_SDCC.h>
#include <stdio.h>

//-----------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

void Port_Init(void);  // Initialize ports for input and output
void Set_Outputs(void);// function to set output bits

//-----------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

__sbit __at 0xB6 LED0; 	 // LED 		@ Port 3 pin 6
__sbit __at 0xB3 BILEDG; // BILED Green @ Port 3 Pin 3
__sbit __at 0xB4 BILEDR; // BILED Red   @ Port 3 Pin 4
__sbit __at 0xB7 BUZZER; // BUZZER 		@ Port 3 Pin 7

__sbit __at 0xA0 SS; 	  // Slide Switch 	@ Port 2 Pin 0
__sbit __at 0xB0 BUTTON1; // Push button 	@ Port 3 Pin 1
__sbit __at 0xB1 BUTTON2; // Push button 	@ Port 3 Pin 0

//-----------------------------------------------------------------------
// Main
//---------------------------------------------------------------------------

void main(void)
{
	Sys_Init(); // System Initialization
	putchar(' '); // because they tell you to
	Port_Init(); // Initialize ports 2 and 3

	while (1) // forevertime loop
	{	
		Set_Outputs(); 
	}
}
 
//-----------------------------------------------------------------------
// Initialize Ports 2 and 3
//---------------------------------------------------------------------------

void Port_Init()
{
	/** Port 3 **/
	P3MDOUT |= 0xd8;  // configure port 3, pins 3,4,6,7 push-pull
	P3MDOUT &= 0xfc;  // configure port 3, pins 0,1 open-drain
	P3 		|= ~0xfc; // set pin impedance on 0,1 high	
	
	/** Port 2 **/
	P2MDOUT &= 0xfe;  // configure port 2, pins 0 open-drain
	P2 		|= ~0xfe; // set pin impedance on 0 
}

//-----------------------------------------------------------------------
// Set the outputs based on program criteria: problem (1)
//---------------------------------------------------------------------------

void Set_Outputs(void)
{
	if (SS) // if slide switch is off
	{
		LED0   = 0; // led is on 	
		BUZZER = 1; // everything else is off
		BILEDR = 1;
		BILEDG = 1;
	}

  	else if (!SS) // if slide switch is activated
	{
		LED0 = 1; //led turned off
		if (!BUTTON1 && BUTTON2) 
		{
			BILEDR = 1;
			BILEDG = 0; // bi-LED green
		}

		else if (BUTTON1 && !BUTTON2)
		{
			BILEDG = 1;
			BILEDR = 0; // bi-LED red
		}

		else if (!BUTTON1 && !BUTTON2)
		{
			BUZZER = 0; // buzzer on
			BILEDG = 1; // bi-LED is off
			BILEDR = 1;
		}

		else 
		{
			BILEDR = 0;
			BILEDG = 0;
			BUZZER = 1; // buzzer off
		}
	}
}