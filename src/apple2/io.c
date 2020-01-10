
#include <6502.h>
#include <apple2.h>
#include <conio.h>
#include "../io.h"
#include "../chargen.h"

const unsigned char colTable = 5;
const unsigned char colLegend = 14;

const unsigned char colBackground = 0;
const unsigned char colBorder = 0;
const unsigned char colText = 5;

const unsigned char colSplash = 4;
const unsigned char colSplashRed = 2;

const unsigned char colTempValue = 11; // temporary roll:
const unsigned char colEvenValue = 5;  // even row roll: green
const unsigned char colOddValue = 13;  // odd row roll: light green
const unsigned char colUpperSum = 4;   // upper sum
const unsigned char colLowerSum = 3;   // lower sum
const unsigned char colBonus = 2;      // bonus
const unsigned char colCurrentRollIdx = 8;

int gDelayTicks;

extern const char *a2dice[6];

void jiffySleep(int num)
{
	unsigned int t;

	for (t = 0; t < gDelayTicks * num; ++t)
		;

}

void startup(void)
{
    clrscr();
    gDelayTicks=30;
    if (get_ostype()>=APPLE_IIGS) {
        cputsxy(0,20,"cortland detected");
        gDelayTicks=72;
    }
}

void initIO(void)
{
    // TODO
}

void initDiceDisplay()
{
    // nothing to be done on PET
}

void __fastcall__ _plotDice(unsigned char value, unsigned char x, unsigned char y, char r)
{

    register unsigned xc;
    register unsigned yc;
    unsigned char idx;
    const char *dicePointer;
    char charIdx = 0;

    if (value == 0)
    {
        for (yc = 0; yc < 3; ++yc)
        {
            cputsxy(x, y + yc, "   ");
        }
        return;
    }

    idx = value - 1;
    dicePointer = a2dice[idx];

    revers(!r);

    for (yc = 0; yc < 3; yc++)
    {
        for (xc = 0; xc < 3; xc++)
        {
            gotoxy(x + xc, y + yc);
            cputc(dicePointer[charIdx++]);
        }
    }

    revers(0);
}

void plotDice(unsigned char nr, unsigned char value, char revers)
{
    _plotDice(value, 35, nr * 5, revers);
}

void eraseDie(unsigned char nr)
{
    _plotDice(0, 35, nr * 5, 0);
}