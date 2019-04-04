
#include "checks.h"

extern unsigned char dvalues[5]; // dice values
extern char numbuf[6];
extern int tvals[18]; // temporary table values (for wizard)

unsigned char checkStreet(void)
{
    unsigned char i;

    for (i = 0; i < 6; i++)
    {
        numbuf[i] = 0;
    }

    for (i = 0; i < 5; i++)
    {
        numbuf[dvalues[i] - 1]++;
    }

    if ((numbuf[0] && numbuf[1] &&
         numbuf[2] && numbuf[3] &&
         numbuf[4]) ||
        (numbuf[1] && numbuf[2] &&
         numbuf[3] && numbuf[4] &&
         numbuf[5]))
    {
        return 5;
    }

    if ((numbuf[0] && numbuf[1] &&
         numbuf[2] && numbuf[3]) ||
        (numbuf[1] && numbuf[2] &&
         numbuf[3] && numbuf[4]) ||
        (numbuf[2] && numbuf[3] &&
         numbuf[4] && numbuf[5]))
    {
        return 4;
    }

    return 0;
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