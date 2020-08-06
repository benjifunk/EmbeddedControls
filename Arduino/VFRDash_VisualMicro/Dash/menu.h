#ifndef __MENU.H
#define __MENU.H

#include "display.h"
#include "Encoder.h"

class shiftlight;

class Menu
{
public:
	Menu();
	void enter();
	shiftlight* writeEEPROM();
	shiftlight* getEEPROM();
	shiftlight* testlights();
	shiftlight* loadallcolors();
	shiftlight* build_segments();
	shiftlight* buildarrays();
protected:
	shiftlight* s;
};

Menu::Menu()
{
}



#endif // !__MENU.H
