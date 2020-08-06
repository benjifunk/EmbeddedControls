#ifndef DISPLAY_H
#define DISPLAY_H

#include <SPI.h>
#include <TFT_ILI9341\TFT_ILI9341.h>

class display :TFT_ILI9341 {
public:

void start(void);
void setFont(uint8_t Text_Color, uint8_t Background, int Font);
void prints(char* );
void reprints(char* );
void setPosition(int , int);
void setFont(int t);
void clear(void);

private:
	int xpos,
		ypos,
		font = 1;
};


#endif // !DISPLAY_H








