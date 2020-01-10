#include <6502.h>
#include <cx16.h>
#include <conio.h>
#include <string.h>
#include "../io.h"
#include "../chargen.h"

#define screen 0
#define COLOR_RAM 2048

void installCharset(void);
void setDiceColor(unsigned char color);

const unsigned char colTable = 5;
const unsigned char colLegend = 14;
const unsigned char colText = 5;

const unsigned char colSplash = COLOR_GREEN;
const unsigned char colSplashRed = COLOR_ORANGE;

const unsigned char colTempValue = COLOR_GRAY2; // temporary roll:
const unsigned char colEvenValue = COLOR_GREEN;  // even row roll: green
const unsigned char colOddValue = COLOR_LIGHTGREEN;  // odd row roll: light green
const unsigned char colUpperSum = COLOR_YELLOW;   // upper sum
const unsigned char colLowerSum = COLOR_PURPLE;   // lower sum
const unsigned char colBonus = COLOR_RED;      // bonus
const unsigned char colCurrentRollIdx = COLOR_YELLOW;

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
    // TODO
}

void initIO(void)
{
    videomode(VIDEOMODE_40COL);
    installCharset();
}

void initDiceDisplay(void)
{
    setDiceColor(COLOR_ORANGE);
}

void setDiceColor(unsigned char color)
{
    unsigned int x, y;
    for (y = 24; y <= 30; ++y)
    {
        for (x = 0; x < 40; ++x)
        {
            vpoke (color,((x*2)+1 + (256 * y)));
        }
    }
}

void _plotDice(unsigned char value, unsigned char x, unsigned char y, char r)
{
    register unsigned int c;
    register unsigned int idx;

    register unsigned int veraCol;

    unsigned int row1, row2, row3, row4, row5;
    idx = value - 1;
    if (r)
        r = 128;

    row1 = screen + (x*2) + (256 * y);
    row2 = row1 + 256;
    row3 = row2 + 256;
    row4 = row3 + 256;
    row5 = row4 + 256;

    if (value == 0)
    {
        for (c = 0; c < 5; ++c)
        {
            veraCol = c*2;
            vpoke(32, row1 + veraCol);
            vpoke(32, row2 + veraCol);
            vpoke(32, row3 + veraCol);
            vpoke(32, row4 + veraCol);
            vpoke(32, row5 + veraCol);
        }
    }
    else
    {
        for (c = 0; c < 5; ++c)
        {
            veraCol = c*2;
            vpoke (r + dice[idx][c], row1+veraCol);
            vpoke (r + dice[idx][c + 5], row2+veraCol);
            vpoke (r + dice[idx][c + 10], row3+veraCol);
            vpoke (r + dice[idx][c + 15], row4+veraCol);
            vpoke (r + dice[idx][c + 20], row5+veraCol);
        }
    }
}

void plotDice(unsigned char nr, unsigned char value, char revers)
{
    _plotDice(value, 3+nr*7, 24, revers);
}

void eraseDie(unsigned char nr)
{
    _plotDice(0, 3+nr*7, 24, 0);
}

void plotDiceLegend(unsigned char flag)
{
	unsigned char i;
	textcolor(colLegend);
	revers(flag);
	for (i = 0; i < 5; ++i)
	{
		gotoxy(2+(i*7), 26);
		if (flag)
		{
			cputc('1' + i);
		}
		else
		{
			cputc(' ');
		}
	}

	revers(0);
}