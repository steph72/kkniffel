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

const unsigned char colBackground = 0;
const unsigned char colText = 1;
const unsigned char colSplash = 2;
const unsigned char colSplashRed = 3;
const unsigned char colTempValue = 4; // temporary roll:
const unsigned char colEvenValue = 5; // even row roll: green
const unsigned char colOddValue = 6;  // odd row roll: light green
const unsigned char colUpperSum = 7;  // upper sum
const unsigned char colLowerSum = 8;  // lower sum
const unsigned char colBonus = 9;     // bonus
const unsigned char colCurrentRollIdx = 10;
const unsigned char colLegend = 11;
const unsigned char colBorder = 12;
const unsigned char colTitleBG = 13;
const unsigned char colTitleText = 14;

void initPalette(void)
{
    cg_setPalette(colBackground, 190, 190, 190);
    cg_setPalette(colBorder, 128, 120, 120);
    cg_setPalette(colText, 0, 0, 0);
    cg_setPalette(colEvenValue, 40, 40, 80);
    cg_setPalette(colOddValue, 30, 30, 30);
    cg_setPalette(colLegend, 0, 0, 120);
    cg_setPalette(colCurrentRollIdx, 0, 20, 30);
    cg_setPalette(colBonus, 140, 0, 0);
    cg_setPalette(colUpperSum, 0, 120, 120);
    cg_setPalette(colLowerSum, 120, 0, 120);
    cg_setPalette(colTempValue, 100, 155, 200);
    cg_setPalette(colTitleBG, 0, 0, 0);
    cg_setPalette(colTitleText, 100, 140, 180);
}

void _plotDice(unsigned char value, unsigned char x, unsigned char y, char r)
{
    register unsigned char xp;
    register unsigned char yp;
    const char *d;
    byte extAttr;

    extAttr = (r == false ? 0 : 16);
    d = &dice[value - 1][0];

    for (yp = 0; yp < 5; yp++)
    {
        for (xp = 0; xp < 8; xp++)
        {
            // cg_plotPetsciiChar(x+xp,y+yp,dtrans[*d++],extAttr);
            cg_plotExtChar(x + xp, y + yp, dtrans[*d++] + extAttr);
        }
    }
}

void plotDice(unsigned char nr, unsigned char value, char revers)
{
    _plotDice(value, DICE_X_POS, nr * 5, revers);
}

void eraseDie(unsigned char nr)
{
    cg_block_raw(DICE_X_POS, nr * 5, DICE_X_POS + 7, (nr * 5) + 4, 32, 0);
}