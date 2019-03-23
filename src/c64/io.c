#include <6502.h>
#include <cbm.h>
#include <conio.h>
#include <c64.h>
#include "../io.h"
#include "../chargen.h"

#define vicpage (unsigned char *)56576u
#define vicctl (unsigned char *)53265u
#define vicadr (unsigned char *)53272u
#define procio (unsigned char *)1u

#define screen (unsigned char *)51200u
#define screenP (unsigned char *)648u // screen pointer for kernel

void installCharset(void);
void setDiceColor(unsigned char color);

const unsigned char colTable = 5;
const unsigned char colLegend = 14;
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

void startup(void)
{
    cbm_k_bsout(0x8e); // select graphic charset
    cbm_k_bsout(0x08); // disable c= + shift
    bgcolor(0);
    bordercolor(0);
    textcolor(5);
    clrscr();
}

void installCharset(void)
{
    unsigned char *adr1;
    unsigned char *adr2;
    unsigned char *b;
    unsigned char i;

    b = (unsigned char *)1024;

    SEI();
    *procio = (*procio & 251);
    adr2 = (unsigned char *)49152u;
    for (adr1 = (unsigned char *)53248u; adr1 <= (unsigned char *)57343u; ++adr1)
    {
        *b = (unsigned char)adr1;
        *adr2 = *adr1;
        ++adr2;
    }
    *procio = (*procio | 4);
    *screenP = (unsigned char)200;

    *vicpage = *vicpage & 252;
    *vicadr = 32;

    for (i = 0; i < 8; i++)
    {
        *((unsigned char *)(49152u + (85 * 8) + i)) = leftUpperCornerC[i];
        *((unsigned char *)(49152u + ((128 + 85) * 8) + i)) = 255 - leftUpperCornerC[i];
        *((unsigned char *)(49152u + (73 * 8) + i)) = rightUpperCornerC[i];
        *((unsigned char *)(49152u + ((128 + 73) * 8) + i)) = 255 - rightUpperCornerC[i];
        *((unsigned char *)(49152u + (74 * 8) + i)) = leftLowerCornerC[i];
        *((unsigned char *)(49152u + ((128 + 74) * 8) + i)) = 255 - leftLowerCornerC[i];
        *((unsigned char *)(49152u + (75 * 8) + i)) = rightLowerCornerC[i];
        *((unsigned char *)(49152u + ((128 + 75) * 8) + i)) = 255 - rightLowerCornerC[i];
    }

    CLI();
}

void initIO(void)
{
    installCharset();
}

void initDiceDisplay(void)
{
    setDiceColor(COLOR_ORANGE);
}

void setDiceColor(unsigned char color)
{
    unsigned int x, y;
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