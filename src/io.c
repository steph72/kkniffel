#include <6502.h>
#include <cbm.h>
#include <conio.h>
#include <c64.h>
#include <string.h>
#include "io.h"
#include "congui.h"

#define fastcall // to silence stupid vscode syntax checker

extern const char dtrans[7];

const char *gTitle = " * KKniffel/MEGA65 * ";

void setDiceColor(unsigned char color);

const unsigned char colTable = 5;
const unsigned char colLegend = 14;
const unsigned char colText = 5;

const unsigned char colSplash = 4;
const unsigned char colSplashRed = 2;

const unsigned char colDice = COLOR_ORANGE;

const unsigned char colTempValue = 11; // temporary roll:
const unsigned char colEvenValue = 5;  // even row roll: green
const unsigned char colOddValue = 13;  // odd row roll: light green
const unsigned char colUpperSum = 4;   // upper sum
const unsigned char colLowerSum = 3;   // lower sum
const unsigned char colBonus = 2;      // bonus
const unsigned char colCurrentRollIdx = 8;

void initDiceDisplay(void)
{
    setDiceColor(COLOR_ORANGE);
}

void _plotDice(unsigned char value, unsigned char x, unsigned char y, char r)
{
    register unsigned char xp;
    register unsigned char yp;
    const char *d;
    byte extAttr;

    extAttr = (r == false ? 0 : 32);
    d = &dice[value-1][0];

    for (yp = 0; yp < 5; yp++)
    {
        for (xp = 0; xp < 5; xp++)
        {
            cg_plotPetsciiChar(x+xp,y+yp,dtrans[*d++],extAttr);
        }
    }
}

void plotDice(unsigned char nr, unsigned char value, char revers)
{
    _plotDice(value, 75, nr * 5, revers);
}

void eraseDie(unsigned char nr)
{
    cg_block_raw(75,nr*5,79,(nr*5)+4,32,0);
}