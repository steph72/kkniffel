/*

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "kcore.h"

#define MAX_ROLL_COUNT 3

int scores[4] = {0, 0, 0, 0};
char sPlayers[4] = {0, 1, 2, 3};

char _isComputerPlayer[4];

char _pname[4][20]; /* player names */
char _numPlayers;
char _numRounds;
char _currentPlayer; /* the current player */
char _currentRound;  /* the current round */
byte _quit;

char numbuf[6];		/* general purpos number buffer */
byte dvalues[5];	/* dice values */
char shouldRoll[5]; /* dice roll flags */

int ktable[18][4][4]; /* main table */

int tvals[18]; /* temporary table values (for wizard) */

byte numRolls;

#ifdef _german_

const char *rownames[] = {"einer", "zweier", "dreier",
						  "vierer", "fuenfer", "sechser",
						  "summe o", "bonus", "gesamt",
						  "3erpasch", "4erpasch", "kl. str.", "gr. str.",
						  "f. house", "kniffel", "scheiss",
						  "summe u", "gesamt"};

#else

const char *rownames[] = {"aces", "twos", "threes",
						  "fours", "fives", "sixes",
						  "sum up", "bonus", "total",
						  "3ofakind", "4ofakind", "sm. str.", "lg. str.",
						  "f. house", "yahtzee", "chance",
						  "sum low", "total"};

#endif

/* -------------------- private functions ------------------------- */

void rollDice(unsigned char nr)
{
	dvalues[nr] = 1 + (rand() % 6);
}

int __fastcall__ dcompare(const void *_a, const void *_b)
{

	unsigned char *a;
	unsigned char *b;
	a = (unsigned char *)_a;
	b = (unsigned char *)_b;

	if (*a < *b)
	{
		return -1;
	}
	else if (*a > *b)
	{
		return 1;
	}
	return 0;
}

unsigned char checkSame(char count)
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
	return false;
}

byte currentDiceSum(void)
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

char checkStraight(void)
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

void updateSums(void)
{
	unsigned char i;
	int upperSum;
	int lowerSum;
	int current;

	upperSum = 0;
	lowerSum = 0;

	for (i = 0; i < 6; ++i)
	{
		current = ktable[i][_currentPlayer][_currentRound];
		if (current >= 0)
		{
			upperSum += current;
		}
	}

	ktable[6][_currentPlayer][_currentRound] = upperSum;

	if (upperSum >= 63)
	{
		ktable[7][_currentPlayer][_currentRound] = 35;
		ktable[8][_currentPlayer][_currentRound] = upperSum + 35;
	}
	else
	{
		ktable[7][_currentPlayer][_currentRound] = -2;
		ktable[8][_currentPlayer][_currentRound] = upperSum;
	}

	for (i = 9; i < 16; i++)
	{
		current = ktable[i][_currentPlayer][_currentRound];
		if (current >= 0)
		{
			lowerSum += current;
		}
	}

	ktable[16][_currentPlayer][_currentRound] = lowerSum;
	ktable[17][_currentPlayer][_currentRound] = lowerSum + ktable[8][_currentPlayer][_currentRound];
}

void nextPlayer(void)
{
	unsigned char i;
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = true;
	}
	++_currentPlayer;
	if (_currentPlayer >= _numPlayers)
	{
		_currentPlayer = 0;
	}
}

/* -------------------- public functions ------------------------- */

/* file handling */

int kc_incrementAndGetSessionCount()
{
	int num;
	FILE *numfile;
	numfile = fopen("kkcnt.dat", "rb");
	if (!numfile)
	{
		num = 0;
	}
	else
	{
		num = fgetc(numfile);
		fclose(numfile);
	}
	numfile = fopen("kkcnt.dat", "wb");
	fputc(++num, numfile);
	fclose(numfile);
	return num;
}

void kc_removeCurrentState(char *fname)
{
	remove(fname);
}

char kc_saveCurrentState(char *fname)
{
	FILE *outfile;
	char idstr[32];
	int i;
	strcpy(idstr, "Kniffel_ST_KKBkc_KSK_01");
	outfile = fopen(fname, "wb");
	if (!outfile)
		return false;
	fwrite(idstr, 32, 1, outfile);
	fputc(_numPlayers, outfile);
	fputc(_numRounds, outfile);
	fputc(_currentPlayer, outfile);
	fputc(_currentRound, outfile);
	fputc(numRolls, outfile);
	fwrite(_pname, sizeof(_pname), 1, outfile);
	fwrite(dvalues, sizeof(dvalues), 1, outfile);
	fwrite(ktable, sizeof(ktable), 1, outfile);
	fwrite(shouldRoll, sizeof(shouldRoll), 1, outfile);
	for (i = 0; i < 4; i++)
	{
		fputc(kc_getIsComputerPlayer(i), outfile);
	}
	fclose(outfile);
	return true;
}

char kc_hasSavedState(char *fname)
{
	FILE *infile;
	infile = fopen(fname, "rb");
	if (infile)
		fclose(infile);
	return (infile != NULL);
}

char kc_loadCurrentState(char *fname)
{
	FILE *infile;
	char cmp[32];
	int i;
	infile = fopen(fname, "rb");
	if (!infile)
		return false;
	fread(cmp, 32, 1, infile);
	_numPlayers = fgetc(infile);
	_numRounds = fgetc(infile);
	_currentPlayer = fgetc(infile);
	_currentRound = fgetc(infile);
	numRolls = fgetc(infile);
	fread(_pname, sizeof(_pname), 1, infile);
	fread(dvalues, sizeof(dvalues), 1, infile);
	fread(ktable, sizeof(ktable), 1, infile);
	fread(shouldRoll, sizeof(shouldRoll), 1, infile);
	for (i = 0; i < 4; i++)
	{
		kc_setIsComputerPlayer(i, fgetc(infile));
	}
	fclose(infile);
	kc_recalcTVals();
	return true;
}

/************ core game functions ***********/

char kc_newRound(void)
{
	if (_currentRound == _numRounds - 1)
	{
		return false;
	}
	_currentRound++;
	return true;
}

void kc_newTurn(void)
{
	char i;
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = true;
		dvalues[i] = 0;
	}
	numRolls = 0;
	nextPlayer();
}

void kc_newRoll(void)
{
	if (numRolls < 3)
		numRolls++;
}

byte kc_getRollCount(void)
{
	return numRolls;
}

void kc_recalcOverallScores(void)
{
	int i, r;
	int current;
	for (i = 0; i < _numPlayers; i++)
	{
		scores[i] = 0;
		for (r = 0; r < 4; r++)
		{
			current = kc_tableValue(17, i, r);
			if (current >= 0)
			{
				scores[i] += current;
			}
		}
	}
}

int scoreComp(const void *_a, const void *_b)
{
	int score1, score2;
	int *a, *b;
	a = (int *)_a;
	b = (int *)_b;
	score1 = scores[*a];
	score2 = scores[*b];
	if (score1 > score2)
		return -1;
	if (score1 < score2)
		return 1;
	return 0;
}

char *kc_sortedPlayers(void)
{
	kc_recalcOverallScores();
	qsort(sPlayers, 4, 1, scoreComp);
	return sPlayers;
}

int kc_getWinner(void)
{
	int i;
	int hs = 0;
	int hp = 0;
	int haveDraw = false;
	for (i = 0; i < _numPlayers; i++)
	{
		if (kc_tableValue(17, i, _currentRound) > hs)
		{
			hs = kc_tableValue(17, i, _currentRound);
			hp = i;
			haveDraw = false;
		}
		else if (kc_tableValue(17, i, _currentRound) == hs)
		{
			haveDraw = true;
		}
	}
	if (haveDraw)
	{
		return -1;
	}
	return hp;
}

char *kc_currentPlayerName(void)
{
	return _pname[_currentPlayer];
}

void kc_recalcTVals(void)
{
	unsigned char row, die, sum;
	unsigned char twoSame;
	unsigned char threeSame;
	unsigned char diceSum;

	for (row = 0; row < 18; row++)
	{
		tvals[row] = 0;
	}

	/* upper section */
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

	/* lower section */

	diceSum = currentDiceSum();
	tvals[15] = diceSum; /* row 15 == chance */

	/* row 14 == kniffel */
	if (checkSame(5))
	{
		tvals[14] = 50;
		tvals[10] = diceSum;
		tvals[9] = diceSum;
	}
	else
	{
		/* row 10 == 4same */
		if (checkSame(4))
		{
			tvals[10] = diceSum;
			tvals[9] = diceSum;
		}
		else
		{
			threeSame = checkSame(3);
			twoSame = checkSame(2);
			/* row 9 == 3same */
			if (threeSame)
			{
				tvals[9] = diceSum;
			}
			/* row 13 == full house */
			if (twoSame && threeSame && (threeSame != twoSame))
			{
				tvals[13] = 25;
			}
			if (!threeSame)
			{
				/* row 12 = lg straight */
				if (checkStraight() == 5)
				{
					tvals[12] = 40;
					tvals[11] = 30;
				}
				else if (checkStraight() == 4)
				{
					/* row 11 = sm straight */
					tvals[11] = 30;
				}
			}
		}
	}
}

char kc_hasChosenRerollDice(void)
{
	unsigned char i;
	unsigned char ret;
	ret = 0;
	for (i = 0; i < 5; i++)
	{
		ret += shouldRoll[i];
	}
	return ret;
}

char kc_canRoll(void)
{
	return kc_hasChosenRerollDice() && (numRolls < 3);
}

void kc_commitSort()
{
	unsigned char i;
	qsort(dvalues, 5, sizeof(unsigned char), dcompare);
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = false;
	}
}

void kc_initGame(int numPlayers, int numRounds)
{
	unsigned char i, j, k;
	for (i = 0; i < 18; ++i)
	{
		tvals[i] = 0;
		for (j = 0; j < 4; ++j)
		{
			scores[j] = 0;
			sPlayers[j] = j;
			for (k = 0; k < 4; ++k)
			{
				ktable[i][j][k] = -1;
			}
		}
	}
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = true;
	}
	_numRounds = numRounds;
	_numPlayers = numPlayers;
	_currentRound = 0;
}

void kc_commitRow(unsigned char row)
{
	char i;
	ktable[row][_currentPlayer][_currentRound] = tvals[row];
	updateSums();
	for (i = 0; i < 18; i++)
	{
		tvals[i] = 0;
	}
}

void kc_debugFill(char num)
{
	char i;
	for (i = 0; i < num; i++)
	{
		ktable[i][_currentPlayer][_currentRound] = rand() % 20;
	}
	updateSums();
}

char *kc_labelForRow(char rowIdx)
{
	return (char *)rownames[rowIdx];
}

char kc_diceValue(char diceIndex)
{
	return dvalues[diceIndex];
}

#ifdef DEBUG
char kc_setDiceValue(char diceIndex, char diceValue)
{
	dvalues[diceIndex] = diceValue;
}
#endif

int kc_tableValue(char row, char player, char round)
{
	return ktable[row][player][round];
}

int kc_tempValue(char row)
{
	return tvals[row];
}

void kc_toggleShouldRoll(unsigned char nr)
{
	shouldRoll[nr] = !shouldRoll[nr];
}

void kc_setShouldRoll(unsigned char nr, unsigned char val)
{
	shouldRoll[nr] = val;
}

char kc_getIsComputerPlayer(int no)
{
	return _isComputerPlayer[no];
}

void kc_setIsComputerPlayer(int no, int isCP)
{
	_isComputerPlayer[no] = isCP;
}

void kc_removeShouldRoll(void)
{
	char i;
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = false;
	}
}

char kc_getShouldRoll(unsigned char nr)
{
	return shouldRoll[nr];
}

void kc_doSingleRoll()
{
	unsigned char i;
	for (i = 0; i < 5; ++i)
	{
		if (shouldRoll[i])
		{
			rollDice(i);
		}
	}
}

char kc_checkQuit()
{
	unsigned char i;
	unsigned char j;
	unsigned char unfinishedEntries;
	unfinishedEntries = 0;
	for (j = 0; j < _numPlayers; j++)
	{
		for (i = 0; i < 18; i++)
		{
			if (ktable[i][j][_currentRound] == -1)
			{
				++unfinishedEntries;
			}
		}
	}
	return (unfinishedEntries == 0);
}

char kc_rowForDataRow(unsigned char dataRow)
{
	if (dataRow <= 5)
	{
		return dataRow + 2;
	}
	if (dataRow >= 6 && dataRow <= 8)
	{
		return dataRow + 3;
	}
	if (dataRow >= 9 && dataRow <= 15)
	{
		return dataRow + 4;
	}
	return dataRow + 5;
}
