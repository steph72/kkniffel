
#include <6502.h>
#include <cbm.h>
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

#define screen (unsigned char *)0x8000u


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
    // TODO
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