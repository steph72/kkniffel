
/*

	CPLAYER
	Computer Player for KKniffel
	Written by Stephan Kleinert, November 2019
	
	Conceived during hours, days and weeks of useless meetings,
	listening to useless overpaid people in suits talk about how they
	want to "develop the app" (even though they have never written
	a single line of code), getting nowhere slowly and maybe someday
	receiving complaints about too many "line breakers" (sic!) in 
	some useless code review.
	
	Oh, the joy of coming home and finally doing some
	real software development...!

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
#include <conio.h>
#include "kcore.h"

int cp_scoreForRowChoice[18];
char cp_sortedRerollRows[18];
char cp_haveBonus;

byte eyesCount[6];

void countDice();
int numOfDiceWith(byte i);

int currentValueForRow(int aRow)
{
	return kc_tableValue(aRow, _currentPlayer, _currentRound);
}

void analyzeUpperRows(void)
{
	char row;
	int currentValue;
	int possibleValue;
	int scoreV;
	char sufficientRoll;

	cp_haveBonus = (currentValueForRow(row_upper_bonus) > 0);

	for (row = 0; row < 6; row++)
	{
		currentValue = currentValueForRow(row);
		if (currentValue == -1)
		{

			possibleValue = tvals[row];
			sufficientRoll = ((possibleValue/(row+1))>=3);

			/* 
					max scoring for upper rows:
			   		30 (6*5) + 50 + 30 = 120			
			*/

			if (cp_haveBonus)
			{
				scoreV = (possibleValue * 4) / 3;
			}
			else
			{
				scoreV = (possibleValue / (row + 1)) * 10;
			}

			/* cannot roll & row not sufficient? rate low! */

			if (kc_getRollCount() == 3)
			{

				if (scoreV < 30)
				{
					cp_scoreForRowChoice[row] = (12 - (row * 2)) + tvals[row];
				}
				else
				{
					cp_scoreForRowChoice[row] = scoreV + possibleValue;
					if (row >= row_3same && !cp_haveBonus)
					{
						// rate high valued rows even higher
						cp_scoreForRowChoice[row] += 30;
					}
				}
			}
			else // can still roll
			{

				if (sufficientRoll)
				{ /* rate 'sufficient' rows higher */
					scoreV += possibleValue*2;
					if (!cp_haveBonus)
					{				  /* ...and even higher if */
						scoreV += 30; /* there is no bonus yet */
					}
				}
				cp_scoreForRowChoice[row] = scoreV;
			}
		}
	}
}

void analyzeSameKind(void)
{
	char row, i;
	char dCountCheck;
	int currentValue;
	int sameDiceValue;
	int scoreV;
	int possibleValue;

	for (row = 9; row < 11; row++)
	{
		currentValue = currentValueForRow(row);
		if (currentValue == -1)
		{
			possibleValue = tvals[row];
			scoreV = 0;
			for (dCountCheck = 2; dCountCheck <= 5; dCountCheck++)
			{
				sameDiceValue = checkSame(dCountCheck);
				if (sameDiceValue)
				{
					scoreV = sameDiceValue * dCountCheck; /* *2 */

					if (row == 9 && dCountCheck >= 3)
					{
						scoreV += 10 + (possibleValue);
						break;
					}
					if (row == 10 && dCountCheck >= 4)
					{
						scoreV += 20 + (possibleValue);
						break;
					}
				}
			}
			cp_scoreForRowChoice[row] = scoreV;
		}
	}
}

int getConsecutiveDiceCount(char **start)
{

	byte i;
	int consec[2];
	static char startElem[2];
	int currentPart;
	int diff;
	unsigned char sorted[5];

	startElem[0] = 0;
	startElem[1] = 0;

	for (i = 0; i < 5; i++)
	{
		sorted[i] = kc_diceValue(i);
	}
	qsort(sorted, 5, 1, dcompare);

	consec[0] = 0;
	consec[1] = 0;
	currentPart = 0;

	for (i = 0; i < 4; i++)
	{
		diff = sorted[i + 1] - sorted[i];
		if (diff == 1)
		{
			if (startElem[currentPart] == 0)
			{
				startElem[currentPart] = sorted[i];
			}
			consec[currentPart]++;
		}
		else if (diff >= 2 && consec[currentPart] != 0)
		{
			currentPart++;
		}
	}
	if (consec[1] > consec[0])
	{
		*start = &startElem[1];
		return consec[1];
	}

	*start = &startElem[0];
	return consec[0];
}

void analyzeStraight(int row)
{
	int currentValue;
	int tempScore = 0;
	int consec;
	char *startElemPtr;

	currentValue = currentValueForRow(row);

	if (currentValue == -1)
	{
		consec = getConsecutiveDiceCount(&startElemPtr);

		if (row == row_sm_straight && consec >= 3)
		{
			if (kc_getRollCount() == 1)
			{ // first roll?

				if (checkSame(2) == 4 &&

					(currentValueForRow(row_fours) == -1 ||
					 currentValueForRow(row_3same) == -1 ||
					 currentValueForRow(row_4same) == -1 ||
					 currentValueForRow(row_kniffel) == -1)

				)
				{
					cp_scoreForRowChoice[row] = 0;
					return;
				}

				if (checkSame(2) == 1 &&

					(currentValueForRow(row_ones) == -1 ||
					 currentValueForRow(row_3same) == -1 ||
					 currentValueForRow(row_4same) == -1 ||
					 currentValueForRow(row_kniffel) == -1)

				)
				{
					cp_scoreForRowChoice[row] = 0;
					return;
				}

				if (checkSame(2) == 2 &&

					(currentValueForRow(row_twos) == -1 ||
					 currentValueForRow(row_3same) == -1 ||
					 currentValueForRow(row_4same) == -1 ||
					 currentValueForRow(row_kniffel) == -1)

				)
				{
					cp_scoreForRowChoice[row] = 0;
					return;
				}
			}
		}

		if (row == 11 && consec >= 3)
		{
			tempScore = 40;
		}
		else if (row == 12 && consec == 3 && kc_getRollCount() < 3)
		{
			tempScore = 45;
		}
		else if (row == 12 && consec == 4)
		{
			tempScore = 42;
		}
		else
		{
			tempScore = consec * 10;
			if (kc_getRollCount() == 2 && tempScore >= 10)
			{
				tempScore -= 10;
			}
			else if (kc_getRollCount() == 3)
			{
				tempScore = 0;
			}
		}
		cp_scoreForRowChoice[row] = tempScore;
	}
}

void analyzeFullHouse(char row)
{
	int currentValue;
	int tempScore = 0;
	int twoSame, threeSame;

	currentValue = currentValueForRow(row);
	if (currentValue == -1)
	{
		threeSame = checkSame(3);
		twoSame = checkSame(2);
		if (twoSame && threeSame && (threeSame != twoSame))
		{
			tempScore = 60;
			if (currentDiceSum() <= 15)
			{
				/* prefer full house with low dice values */
				tempScore += 20;
			}
		}
		else if (threeSame && !twoSame)
		{
			if (kc_getRollCount() != 3)
			{
				tempScore = 40;
			}
			else
			{
				tempScore = 5;
			}
		}
		cp_scoreForRowChoice[row] = tempScore;
	}
}

void analyzeKniffel(int row)
{
	int currentValue;
	int same;
	int i;
	int score;

	cp_scoreForRowChoice[row] = 0;

	currentValue = currentValueForRow(row);
	if (currentValue >= 0)
	{
		return;
	}

	for (i = 5; i > 2; i--)
	{
		same = checkSame(i);
		if (same)
		{
			if (i == 5)
			{
				cp_scoreForRowChoice[row] = 200;
				if (tvals[row] == 30)
				{
					if (!cp_haveBonus && (currentValueForRow(row_sixes) < 0))
					{
						cp_scoreForRowChoice[row] = 0;
					}
				}
				if (tvals[row] == 25)
				{
					if (!cp_haveBonus && (currentValueForRow(row_fives) < 0))
					{
						cp_scoreForRowChoice[row] = 0;
					}
				}
				if (tvals[row] == 20)
				{
					if (!cp_haveBonus && (currentValueForRow(row_fours) < 0))
					{
						cp_scoreForRowChoice[row] = 0;
					}
				}
				return;
			}
			score = 10 * i;
			score += (3 - kc_getRollCount()) * 10;
			cp_scoreForRowChoice[row] = score;
		}
	}
}

int scoreSort(const void *_a, const void *_b)
{
	int score1;
	int score2;
	char *a, *b;
	a = (char *)_a;
	b = (char *)_b;
	score1 = cp_scoreForRowChoice[*a];
	score2 = cp_scoreForRowChoice[*b];
	if (score1 < score2)
		return 1;
	if (score1 > score2)
		return -1;
	if (*a > *b)
		return 1;
	if (*a < *b)
		return -1;
	return 0;
}

int markDiceForUpperSection(int aRow)
{
	byte i;
	int dValue = aRow + 1;
	int currentValue;
	currentValue = tvals[aRow];
	if (currentValue == ((aRow + 1) * 5))
	{
		return aRow;
	}
	if (currentValue >= ((aRow + 1) * 3) && kc_getRollCount() == 3)
	{
		return aRow;
	}
	if (kc_getRollCount() == 3)
	{
		return -1;
	}
	for (i = 0; i < 5; i++)
	{
		kc_setShouldRoll(i, kc_diceValue(i) != dValue);
	}
	return -1;
}

int markDiceForSame(int row)
{
	int destCount = 0;
	int testCount = 0;
	int destI = 0;
	byte i;

	if (tvals[row] > 0)
	{
		return row;
	}

	countDice();

	for (i = 1; i <= 6; i++)
	{
		testCount = numOfDiceWith(i);
		if (testCount > destCount)
		{
			destCount = testCount;
			destI = i;
		}
	}

	for (i = 0; i < 5; ++i)
	{
		if (destCount == 1)
		{
			kc_setShouldRoll(i, true);
		}
		else
		{
			kc_setShouldRoll(i, kc_diceValue(i) != destI);
		}
	}

	return -1;
}

int markDiceForFullHouse(void)
{
	int valToKeep;
	byte i;
	int currentValue;
	currentValue = tvals[row_full_house];
	if (currentValue > 0)
	{
		return row_full_house;
	}
	valToKeep = checkSame(3);
	if (!valToKeep)
	{
		valToKeep = checkSame(2);
	}

	/* mark all dice that are not in found 2 or 3) */
	for (i = 0; i < 5; ++i)
	{
		kc_setShouldRoll(i, kc_diceValue(i) != valToKeep);
	}

	/* we need one less to roll in any case */

	for (i = 0; i < 5; i++)
	{
		if (kc_getShouldRoll(i))
		{
			kc_toggleShouldRoll(i);
			break;
		}
	}
	return -1;
}

void markDiceWithValue(char val)
{
	char i;
	for (i = 0; i < 5; ++i)
	{
		if (kc_diceValue(i) == val)
		{
			kc_setShouldRoll(i, true);
		}
	}
}

void markFirstDiceWithValue(char val)
{
	char i;
	for (i = 0; i < 5; ++i)
	{
		if (kc_diceValue(i) == val)
		{
			kc_setShouldRoll(i, true);
			return;
		}
	}
}

int markDiceForStraight(int row)
{
	char diceValueCount[6];
	char currentDiceVal;
	char i;
	char hasDoubles;
	char currentValue;
	currentValue = tvals[row];
	if (row >= row_4same && currentValue > 0)
	{
		return row;
	}
	if (row == row_3same && kc_getRollCount() == 3)
	{
		return row;
	}

	memset(diceValueCount, 0, 6);

	/* sm straight is one of

	x1234
	x2345
	x3456

	*/

	// try for double dice values
	hasDoubles = false;
	for (i = 0; i < 5; i++)
	{
		currentDiceVal = kc_diceValue(i);
		diceValueCount[currentDiceVal - 1]++;
		if (diceValueCount[currentDiceVal - 1] > 1)
		{
			hasDoubles = true;
			kc_setShouldRoll(i, true);
		}
	}

	if (hasDoubles)
	{
		return -1;
	}

	if (diceValueCount[0] == 1 && diceValueCount[1] == 1 && diceValueCount[5] == 1)
	{ // x1234 : x==6
		markDiceWithValue(6);
	}
	else
	{ // x3456 : x==1
		markDiceWithValue(1);
	}

	return -1;
}

int markDiceSK(void)
{
	int row;
#ifdef DEBUG
	gotoxy(0, 0);
	cputs("markd sk ");
#endif
	row = cp_sortedRerollRows[0];
	kc_recalcTVals();

	if (cp_scoreForRowChoice[row] == -1)
	{
		// absolutely no clue what to mark? -> fall back to 4same
		cputs("ones!");
		return markDiceForSame(row_4same);
	}
	if (row <= 5 && row >= 0)
	{
#ifdef DEBUG
		cputc('1' + row);
#endif
		return markDiceForUpperSection(row);
	}
	else if (row == row_kniffel || row == row_3same || row == row_4same)
	{
#ifdef DEBUG
		cputs("same");
		cputc('1' + row);
#endif
		return markDiceForSame(row);
	}
	else if (row == row_sm_straight || row == row_lg_straight)
	{
#ifdef DEBUG
		cputs("straight");
		cputc('1' + row);
#endif
		return markDiceForStraight(row);
	}
	else if (row == row_full_house)
	{
#ifdef DEBUG
		cputs("fullh");
		cputc('1' + row);
#endif
		return markDiceForFullHouse();
	}
	return markDiceForSame(row_4same);
}

int numOfDiceWith(byte i)
{
	return eyesCount[i - 1];
}

void countDice()
{
	byte i;
	byte res;
	for (i = 0; i < 6; i++)
	{
		eyesCount[i] = 0;
	}

	for (i = 0; i < 5; i++)
	{
		res = kc_diceValue(i);
		++eyesCount[res - 1];
	}
}

// clang-format off
#pragma warn(unused-param, push, off)
void logRule(byte i)
{
#ifdef DEBUG
    // gotoxy(1, 23);
	// cprintf("r%d ", i);
#endif
}
#pragma warn(unused-param, pop)
// clang-format on

int markDiceWP()
{
	byte i, j;
	byte res;

#ifdef DEBUG
	gotoxy(0, 0);
	cputs("markd wp");
#endif

	countDice();
	kc_recalcTVals();

	if (tvals[row_lg_straight] != 0)
	{
		return row_lg_straight;
	}

	if (kc_getRollCount() == 1) // -------------- exceptions before second roll ----------------
	{
		// === RULE 1 ===
		// with a full house containing three 1s, he will keep all the dice and
		// put in the Full House box.
		if (checkSame(3) && checkSame(2) && (checkSame(3) != checkSame(2)) && eyesCount[0] == 3)
		{
			logRule(1);
			if (currentValueForRow(row_full_house) == -1)
			{
				return row_full_house;
			}
		}

		// === RULE 2 ===
		// with 12344 he will keep the pair not the small straight.
		if (numOfDiceWith(1) == 1 &&
			numOfDiceWith(2) == 1 &&
			numOfDiceWith(3) == 1 &&
			numOfDiceWith(4) == 2)
		{
			logRule(2);
			markDiceWithValue(1);
			markDiceWithValue(2);
			markDiceWithValue(3);
			return -1;
		}

		// === RULE 3 ===
		// with a 3456 and a pair he will keep the pair not the small straight.
		if (numOfDiceWith(3) >= 1 &&
			numOfDiceWith(4) >= 1 &&
			numOfDiceWith(5) >= 1 &&
			numOfDiceWith(6) >= 1 &&
			(numOfDiceWith(3) == 2 ||
			 numOfDiceWith(4) == 2 ||
			 numOfDiceWith(5) == 2 ||
			 numOfDiceWith(6) == 2))
		{
			logRule(3);
			for (i = 1; i <= 6; i++)
			{
				if (numOfDiceWith(i) == 1)
				{
					markDiceWithValue(i);
					return -1;
				}
			}
		}

		// === RULE 4 ===
		// with a pair of 1s he will keep, in order of preference,
		// 345, 5, 4 or 6, not the pair of 1s.

		if (numOfDiceWith(1) == 2)
		{
			if (numOfDiceWith(3) == 1 &&
				numOfDiceWith(4) == 1 &&
				numOfDiceWith(5) == 1)
			{
				logRule(41);
				markDiceWithValue(1);
				return -1;
			}
			if (numOfDiceWith(5) >= 1)
			{
				logRule(42);
				markDiceWithValue(1);
				markDiceWithValue(2);
				markDiceWithValue(3);
				markDiceWithValue(4);
				markDiceWithValue(6);
				return -1;
			}
			if (numOfDiceWith(4) >= 1)
			{
				logRule(43);
				markDiceWithValue(1);
				markDiceWithValue(2);
				markDiceWithValue(3);
				markDiceWithValue(5);
				markDiceWithValue(6);
				return -1;
			}
			if (numOfDiceWith(6) >= 1)
			{
				logRule(44);
				markDiceWithValue(1);
				markDiceWithValue(2);
				markDiceWithValue(3);
				markDiceWithValue(4);
				markDiceWithValue(5);
				return -1;
			}
		}
	}
	else //  -------------- exceptions before second roll ----------------
	{

		// === RULE 5 ===
		// with a full house containing three 1s, three 2s or three 3s,
		// he will keep all the dice and put in the Full House box.

		if ((checkSame(3) && checkSame(2) && (checkSame(3) != checkSame(2))) &&
			(numOfDiceWith(3) == 3 || numOfDiceWith(2) == 3 || numOfDiceWith(1) == 3))
		{
			if (currentValueForRow(row_full_house) == -1)
			{
				logRule(5);
				return row_full_house;
			}
		}

		// === RULE 6 & 7 ===
		// with a pair of 1s and a pair of 2s
		// or with a pair of 1s and a pair of 3s,
		// he will keep both pairs.

		if (numOfDiceWith(1) == 2 && numOfDiceWith(2) == 2)
		{
			logRule(6);
			markDiceWithValue(3);
			markDiceWithValue(4);
			markDiceWithValue(5);
			markDiceWithValue(6);
			return -1;
		}

		if (numOfDiceWith(1) == 2 && numOfDiceWith(3) == 2)
		{
			logRule(7);
			markDiceWithValue(2);
			markDiceWithValue(4);
			markDiceWithValue(5);
			markDiceWithValue(6);
			return -1;
		}

		// === RULE 8 ===
		// with 11345 he will keep 345 not the pair.

		if (numOfDiceWith(1) == 2 && numOfDiceWith(3) == 1 &&
			numOfDiceWith(4) == 1 && numOfDiceWith(5) == 1)
		{
			logRule(8);
			markDiceWithValue(1);
			return -1;
		}

		// === RULE 9 ===
		// with 12456 he will keep 456 not just the 5.

		if (numOfDiceWith(1) == 1 &&
			numOfDiceWith(2) == 1 &&
			numOfDiceWith(4) == 1 &&
			numOfDiceWith(5) == 1 &&
			numOfDiceWith(6) == 1)
		{
			logRule(9);
			markDiceWithValue(1);
			markDiceWithValue(2);
			return -1;
		}
	}

	// regular rules (applied if special cases don't apply)

	// === RULE 10 ===
	if (checkSame(4))
	{
		logRule(10);
		markDiceWithValue(checkSame(1));
		return -1;
	}

	// === RULE 11 ===
	// When a player has a full house he will keep the three-of-a-kind.
	if (checkSame(3))
	{
		logRule(11);
		for (i = 0; i < 5; i++)
		{
			if (kc_diceValue(i) != checkSame(3))
			{
				kc_setShouldRoll(i, true);
			}
		}
		return -1;
	}

	// === RULE 12 ===
	// With two pairs, keep the higher pair and rethrow the other three dice.
	// 25251
	res = 0;
	for (i = 1; i <= 6; i++)
	{
		if (numOfDiceWith(i) == 2)
		{
			res++;
		}
	}
	if (res == 2)
	{
		logRule(12);
		for (i = 6; i >= 1; i--)
		{
			if (numOfDiceWith(i) == 2)
			{
				for (j = 1; j <= 6; j++)
				{
					if (j != i)
					{
						markDiceWithValue(j);
					}
				}
				return -1;
			}
		}
	}

	// === RULE 13 ===
	// If a large straight is rolled, keep it.

	if (
		(numOfDiceWith(1) == 1 &&
		 numOfDiceWith(2) == 1 &&
		 numOfDiceWith(3) == 1 &&
		 numOfDiceWith(4) == 1 &&
		 numOfDiceWith(5) == 1) ||
		((numOfDiceWith(2) == 1 &&
		  numOfDiceWith(3) == 1 &&
		  numOfDiceWith(4) == 1 &&
		  numOfDiceWith(5) == 1 &&
		  numOfDiceWith(6) == 1)))
	{
		logRule(13);
		if (currentValueForRow(row_full_house) == -1)
		{
			return row_lg_straight;
		}
	}

	// === RULE 14 ===
	//  If a small straight is rolled, keep it and re-roll the fifth die.

	if (currentValueForRow(row_lg_straight) == -1)
	{
		if (tvals[row_sm_straight] == 30)
		{
			logRule(14);
			/* sm straight is one of

	x1234
	x2345
	x3456

	*/

			// try for double dice values
			for (i = 1; i <= 6; i++)
			{
				if (numOfDiceWith(i) == 2)
				{
					markFirstDiceWithValue(i);
					return -1;
				}
			}

			if (numOfDiceWith(1) == 1 && numOfDiceWith(2) == 1 && numOfDiceWith(6) == 1)
			{ // x1234 : x==6
				markDiceWithValue(6);
			}
			else
			{ // x3456 : x==1
				markDiceWithValue(1);
			}
		}
	}

	// === RULE 15 ===
	// one pair and nothing else: keep the pair
	if (checkSame(2))
	{
		logRule(15);
		for (i = 0; i < 5; i++)
		{
			if (kc_diceValue(i) != checkSame(2))
			{
				kc_setShouldRoll(i, true);
			}
		}
		return -1;
	}

	// === RULE 16 ===
	//  If all the dice are different and there is no straight, keep only the 5.

	if (tvals[row_sm_straight] == 0 && tvals[row_lg_straight] == 0)
	{
		logRule(16);
		if (numOfDiceWith(5) >= 1)
		{
			markDiceWithValue(1);
			markDiceWithValue(2);
			markDiceWithValue(3);
			markDiceWithValue(4);
			markDiceWithValue(6);
			return -1;
		}
	}

	return -1;
}

int cp_markDice()
{

	if (kc_getNumTurnsForPlayer(_currentPlayer) <= 3)
	{
		return markDiceWP();
	}
	else
	{
		return markDiceSK();
	}
}

void cp_analyze(void)
{
	int i;
	for (i = 0; i < 18; i++)
	{
		cp_sortedRerollRows[i] = i;
		cp_scoreForRowChoice[i] = -1;
	}
	kc_recalcTVals();
	analyzeUpperRows();
	analyzeSameKind();
	analyzeStraight(11);
	analyzeStraight(12);
	analyzeFullHouse(13);
	analyzeKniffel(14);
	qsort(cp_sortedRerollRows, 18, 1, scoreSort);
#ifdef DEBUG
	gotoxy(0, 23);
	cprintf("keep: %c", kc_letterForRow(cp_sortedRerollRows[0]));
#endif
	/* debugDumpCP(); */
}

int cp_exitRow(void)
{

	int i;
	int currentValue;
	int upperSum = 0;
	int pointsNeededForBonus = 0;
	int noGoodRoll;

	/* determine if we have all upper rolls... */
	for (i = 0; i <= 5; i++)
	{
		currentValue = currentValueForRow(i);
		if (currentValue >= 0)
		{
			upperSum += currentValue;
		}
	}
	pointsNeededForBonus = 63 - upperSum;

	/* always leave chance possible... */
	if (currentValueForRow(row_chance) < 0)
	{
		cp_scoreForRowChoice[row_chance] = tvals[row_chance];
	}

	qsort(cp_sortedRerollRows, 18, 1, scoreSort);

	noGoodRoll = (cp_sortedRerollRows[0] == row_chance && tvals[row_chance] < 20);
	noGoodRoll |= (cp_sortedRerollRows[0] == row_3same && tvals[row_3same] < 12);
	noGoodRoll |= (cp_sortedRerollRows[0] == row_4same && tvals[row_4same] < 16);
	noGoodRoll |= (cp_sortedRerollRows[0] == row_sixes && tvals[row_sixes] < 18);
	noGoodRoll |= (cp_sortedRerollRows[0] == row_fives && tvals[row_fives] < 15);
	noGoodRoll |= (cp_sortedRerollRows[0] == row_fours && tvals[row_fours] < 12);
	noGoodRoll |= (cp_sortedRerollRows[0] == row_threes && tvals[row_threes] < 9);
	noGoodRoll |= (cp_sortedRerollRows[0] == row_kniffel && tvals[row_kniffel] == 0);

	// use ones or twos as alternative chance if chance is bad
	if (noGoodRoll)
	{
		if (currentValueForRow(row_ones) < 0)
		{
			return row_ones;
		}
		if (currentValueForRow(row_chance) < 0)
		{
			return row_chance;
		}
		if (currentValueForRow(row_twos) < 0)
		{
			return row_twos;
		}
	}

	// zeroing something important?

	if (tvals[cp_sortedRerollRows[0]] == 0)
	{

		// no bonus?
		if (pointsNeededForBonus > 0)
		{
			// possible to score something in the lower rows?
			// --> take that row instead
			for (i = row_3same; i <= row_chance; ++i)
			{
				if ((currentValueForRow(i) < 0) && tvals[i] > 0)
				{
					return i;
				}
			}
		}
		else
		{ // already have upper bonus

			for (i = row_ones; i <= row_sixes; ++i)
			{
				// something free in upper rows -> take it
				if (currentValueForRow(i) < 0)
				{
					return i;
				}
			}
		}
	}

	return cp_sortedRerollRows[0];
}

#ifdef DEBUG
void debugDumpChoices()
{
	char i;
	int earlyExitRow, exitRow;

	textcolor(7);
	cp_analyze();
	earlyExitRow = cp_markDice();
	gotoxy(0, 0);
	for (i = 0; i < 18; ++i)
	{
		gotoxy(12, kc_rowForDataRow(i));
		printf("%d ", cp_scoreForRowChoice[i]);
	}
	exitRow = cp_exitRow();
	for (i = 0; i < 18; ++i)
	{
		gotoxy(15, kc_rowForDataRow(i));
		printf("%d ", cp_scoreForRowChoice[i]);
	}
	gotoxy(10, 23);
	if (earlyExitRow != -1)
	{
		revers(1);
		printf(" !! ee: %d !! ", earlyExitRow);
		revers(0);
	}
	else
	{
		printf("ee: %d,  e: %d  ", earlyExitRow, exitRow);
	}
}
#endif