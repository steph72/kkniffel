
#include <6502.h>
#include <cbm.h>
#include <conio.h>
#include <plus4.h>

#include "../io.h"
#include "../chargen.h"

const char *gTitle = " * kkniffel/ted * ";


const unsigned char colBackground = BCOLOR_BLACK;
const unsigned char colBorder = BCOLOR_BLACK;
const unsigned char colText = BCOLOR_BLUEGREEN + CATTR_LUMA4;

const unsigned char colSplash = BCOLOR_PURPLE + CATTR_LUMA3;
const unsigned char colSplashRed = BCOLOR_RED + CATTR_LUMA3;

const unsigned char colDice = COLOR_ORANGE;


const unsigned char colTable = BCOLOR_CYAN + CATTR_LUMA0;
const unsigned char colLegend = BCOLOR_LIGHTBLUE + CATTR_LUMA3;

const unsigned char colTempValue = BCOLOR_WHITE + CATTR_LUMA1 ; // temporary roll:
const unsigned char colEvenValue = BCOLOR_GREEN + CATTR_LUMA3;  // even row roll: green
const unsigned char colOddValue = BCOLOR_GREEN + CATTR_LUMA4;  // odd row roll: light green
const unsigned char colUpperSum = BCOLOR_YELLOW + CATTR_LUMA5;   // upper sum
const unsigned char colLowerSum = BCOLOR_YELLOW + CATTR_LUMA5;   // lower sum
const unsigned char colBonus = BCOLOR_RED + CATTR_LUMA3;      // bonus
const unsigned char colCurrentRollIdx = BCOLOR_ORANGE + CATTR_LUMA4; 

#define screen (unsigned char *)0x0c00

void setDiceColor(unsigned char color);

void startup(void)
{
    cbm_k_bsout(0x8e); // select graphic charset
    cbm_k_bsout(0x08); // disable c= + shift
    bgcolor(colBackground);
    bordercolor(colBorder);
    textcolor(colText);
    clrscr();
}

void initIO(void)
{
    // TODO: install custom charset for dice
}

void initDiceDisplay(void)
{
    setDiceColor(BCOLOR_ORANGE + CATTR_LUMA4);
}


void setDiceColor(unsigned char color)
{
    unsigned int x,y;
    for (y = 0; y <= 24; ++y)
    {
        for (x = 35; x < 40; ++x)
        {
                *(COLOR_RAM + x + (40 * y)) = color;
        }
    }
}

void __fastcall__ _plotDice(unsigned char value, unsigned char x, unsigned char y, char r)
{
    register unsigned char c;
    register unsigned char idx;

    unsigned char *row1, *row2, *row3, *row4, *row5;
    idx = value - 1;
    if (r)
        r = 128;

    row1 = (unsigned char *)screen + x + (40 * y);
    row2 = (unsigned char *)row1 + 40;
    row3 = (unsigned char *)row2 + 40;
    row4 = (unsigned char *)row3 + 40;
    row5 = (unsigned char *)row4 + 40;

    if (value == 0)
    {
        for (c = 0; c < 5; ++c)
        {
            row1[c] = 32;
            row2[c] = 32;
            row3[c] = 32;
            row4[c] = 32;
            row5[c] = 32;
        }
    }
    else
    {
        for (c = 0; c < 5; ++c)
        {
            row1[c] = r + dice[idx][c];
            row2[c] = r + dice[idx][c + 5];
            row3[c] = r + dice[idx][c + 10];
            row4[c] = r + dice[idx][c + 15];
            row5[c] = r + dice[idx][c + 20];
        }
    }
}

void plotDice(unsigned char nr, unsigned char value, char revers)
{
    _plotDice(value, 35, nr * 5, revers);
}

void eraseDie(unsigned char nr)
{
    _plotDice(0, 35, nr * 5, 0);
}