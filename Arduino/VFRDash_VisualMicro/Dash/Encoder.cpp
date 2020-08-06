#include "Encoder.h"

int menuvar;
int val;
int rotaryval = 0;

volatile unsigned char state = 0;



Encoder::Encoder()
{
	pinMode(button_pin, INPUT_PULLUP);
	pinMode(ROTARY_PIN1, INPUT_PULLUP);
	pinMode(ROTARY_PIN2, INPUT_PULLUP);
}

Encoder::~Encoder()
{
}

char Encoder::rotary_process() {
	char pinstate = (digitalRead(ROTARY_PIN2) << 1) | digitalRead(ROTARY_PIN1);
	state = ttable[state & 0xf][pinstate];
	return (state & 0xc0);
}

bool Encoder::button_pushed(){
	if (digitalRead(button_pin) == LOW) return true;//use: if(Encoderobj.button_pushed()) Menuobj.enter();
}