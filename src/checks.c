
#include "checks.h"
#include <conio.h>
#include <stdlib.h>

extern unsigned char dvalues[5]; // dice values
extern char numbuf[6];
extern int tvals[18]; // temporary table values (for wizard)

extern int ktable[18][4];  // main table
extern char currentPlayer; // the current player

int __fastcall__ dcompare(const void *_a, const void *_b);

unsigned char checkStreet(void)
{
    char i;
    char count;

    for (i = 0; i < 5; ++i)
    {
        numbuf[i] = dvalues[i];
    }

    qsort(numbuf, 5, sizeof(unsigned char), dcompare);

    count = 0;
    for (i = 0; i < 4; ++i)
    {
        if (numbuf[i + 1] == numbuf[i] + 1)
        {
            ++count;
        }
    }
    return count + 1;
}

unsigned char currentDiceSum(void)
{
    unsigned char die;
    unsigned char sum;
    sum = 0;
    for (die = 0; die < 5; ++die)
    {
        sum += dvalues[die];
    }
    return sum;
}

unsigned char checkSame(unsigned char count)
{
    unsigned char die;
    unsigned char number;
    unsigned char occurances;
    for (number = 1; number < 7; ++number)
    {
        occurances = 0;
        for (die = 0; die < 5; ++die)
        {
            if (dvalues[die] == number)
            {
                ++occurances;
            }
        }
        if (occurances == count)
        {
            return number;
        }
    }
    return FALSE;
}

unsigned char checkDiceCountWithDigit(unsigned char digit)
{
    unsigned char i;
    unsigned char occurances = 0;
    for (i = 0; i < 5; ++i)
    {
        if (dvalues[i] == digit)
            occurances++;
    }
    return occurances;
}

unsigned char rerollDiceCountForRow(unsigned char row)
{
    int i;
    signed char v;

    if (row <= 5) // upper rows
    {
        return 5 - (checkDiceCountWithDigit(row + 1));
    }

    if (row == 9 || row == 10) // x same?
    {
        v = row - 6;

        for (i = v; i > 0; i--)
        {
            if (checkSame(i))
            {
                return v - i;
            }
        }
    }

    if (row == 11) // sm. straight?
    {
        v = 4 - checkStreet();
        if (v > 0)
        {
            return v;
        }
        else
        {
            return 0;
        }
    }

    if (row == 12) // lg. straight?
    {
        v = 5 - checkStreet();
        if (v > 0)
        {
            return v;
        }
        else
        {
            return 0;
        }
    }

    if (row == 13) // full house
    {

        if (checkSame(3) && checkSame(2))
        {
            return 0;
        }
        else if (checkSame(3))
        {
            return 1;
        }
        else if (checkSame(2))
        {
            return 3;
        }
        else
        {
            return 3;
        }
    }

    if (row == 14) // kniffel
    {
        for (i = 5; i > 0; i--)
        {
            if (checkSame(i))
            {
                return 5 - i;
            }
        }
    }

    return 0;
}

signed char determineUpperRerollRow(void)
{
    signed char row,i;
    char min, test;
    
    min=255;
    row=-1;

    for (i = 5; i >= 0; --i)
    {
        if (ktable[i][currentPlayer] < 0)
        {
            cprintf("+");
            test = rerollDiceCountForRow(i);
            cprintf("%d",test);
            if (test<=min) {
                cprintf("m");
                min = test;
                row = i;
            }
        }
    }

    return row;

}

void recalcTVals(void)
{
    unsigned char row, die, sum;
    unsigned char twoSame;
    unsigned char threeSame;
    unsigned char diceSum;

    for (row = 0; row < 18; row++)
    {
        tvals[row] = 0;
    }

    // upper section
    for (row = 0; row < 6; row++)
    {
        sum = 0;
        for (die = 0; die < 5; die++)
        {
            if (dvalues[die] == (row + 1))
            {
                sum += dvalues[die];
            }
        }
        tvals[row] = sum;
    }

    // lower section

    diceSum = currentDiceSum();
    tvals[15] = diceSum; // row 15 == chance

    // row 14 == kniffel
    if (checkSame(5))
    {
        tvals[14] = 50;
        tvals[10] = diceSum;
        tvals[9] = diceSum;
    }
    else
    {
        // row 10 == 4same
        if (checkSame(4))
        {
            tvals[10] = diceSum;
            tvals[9] = diceSum;
        }
        else
        {
            threeSame = checkSame(3);
            twoSame = checkSame(2);
            // row 9 == 3same
            if (threeSame)
            {
                tvals[9] = diceSum;
            }
            // row 13 == full house
            if (twoSame && threeSame && (threeSame != twoSame))
            {
                tvals[13] = 25;
            }
            if (!threeSame)
            {
                // row 12 = lg street
                if (checkStreet() == 5)
                {
                    tvals[12] = 40;
                    tvals[11] = 30;
                }
                else if (checkStreet() == 4)
                {
                    // row 11 = sm street
                    tvals[11] = 30;
                }
            }
        }
    }
}