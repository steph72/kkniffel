
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <6502.h>
#include <time.h>

#include "io.h"

#define FALSE 0
#define TRUE -1

#define MAX_ROLL_COUNT 3

// clang-format off
#if defined(__PET__)
#pragma warn(no-effect, off)
#endif
// clang-format on

char inbuf[40];
char numbuf[6];
char pnames[4][20];		  // player names
unsigned char dvalues[5]; // dice values
char shouldRoll[5];		  // dice roll flags

int ktable[18][4];		 // main table
int roundResults[10][4]; // results for postgame
int totals[4];			 // totals per player for postgame

char currentPlayer; // the current player

int tvals[18]; // temporary table values (for wizard)

void recalcTVals(void);		  // recalc temp values
void removeTvalDisplay(void); // remove tval display

unsigned char quit;
unsigned char currentRound;

const char *rownames[] = {"einer", "zweier", "dreier", "vierer", "fuenfer",
						  "sechser", "summe o", "bonus", "gesamt o", "3erpasch", "4erpasch",
						  "kl. str", "gr. str", "f. house", "kniffel", "chance",
						  "summe u", "gesamt"};

char numPlayers;
char namelength;

void clearLower()
{
	gotoxy(0, 24);
	cputs("                                  ");
}

void centerLower(char *msg)
{
	clearLower();
	gotoxy(17 - (strlen(msg) / 2), 24);
	cputs(msg);
	gotoxy(0, 0);
}

void clearbuf()
{
	while (kbhit())
		cgetc();
}

void waitkey(char key)
{
	clearbuf();
	while (cgetc() != key)
		;
}

void rollDice(unsigned char nr)
{
	dvalues[nr] = 1 + (rand() % 6);
	if (dvalues[nr] < 5)
		return;
}

void plotDiceLegend(unsigned char flag)
{
	unsigned char i;
	textcolor(colLegend);
	revers(flag);
	for (i = 0; i < 5; ++i)
	{
		gotoxy(34, (i * 5) + 2);
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

void doSingleRoll()
{
	unsigned char i;
	for (i = 0; i < 5; ++i)
	{
		if (shouldRoll[i])
		{
			rollDice(i);
			plotDice(i, dvalues[i], FALSE);
		}
	}
}

void showCurrentRoll()
{
	unsigned char i;
	for (i = 0; i < 5; i++)
	{
		plotDice(i, dvalues[i], 0);
	}
}

void eraseDice()
{
	unsigned char i;
	revers(0);
	for (i = 0; i < 5; i++)
	{
		eraseDie(i);
		gotoxy(34, (i * 5) + 2);
		cputc(' ');
	}
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

void doTurnRoll()
{
	char i;
	clearLower();
	removeTvalDisplay();
	showCurrentRoll();
	clearbuf();
	do
	{
		doSingleRoll();
	} while (kbhit() == 0);
	doSingleRoll();
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = FALSE;
	}
	recalcTVals();
	clearbuf();
}

void input(char *buf)
{
	char currentChar;
	char currentPos;
	currentPos = 0;
	buf[0] = 0;
	cursor(1);
	do
	{
		currentChar = cgetc();
		if (currentChar != '\n' && currentChar != 20)
		{
			buf[currentPos] = currentChar;
			cputc(currentChar);
			currentPos++;
		}
		if (currentChar == 20 && currentPos > 0)
		{
			currentPos--;
			gotox(wherex() - 1);
			cputc(' ');
			gotox(wherex() - 1);
		}
	} while (currentChar != '\n');
	buf[currentPos] = 0;
	cursor(0);
}

char textcolorForRow(char i)
{
	if (i == 7)
	{
		return colBonus;
	}
	if (i == 6 || i == 8)
	{
		return colUpperSum;
	}
	if (i == 16 || i == 17)
	{
		return colLowerSum;
	};

	if (i % 2)
	{
		return colEvenValue;
	}
	else
	{
		return colOddValue;
	}
	return 0;
}

char columnForPlayer(unsigned char p)
{
	return (12 + (p * (namelength + 1)));
}

char rowForDataRow(unsigned char dataRow)
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

void displayTableEntry(char player, char row, int value, char temp)
{
	char color;
	if (temp)
	{
#if defined(__PET__)
		revers(value > 0);
#else
		color = colTempValue;
#endif
	}
	else
	{
#if defined(__PET__)
		revers(0);
#else
		color = textcolorForRow(row);
#endif
	}
	textcolor(color);
	itoa(value, numbuf, 10);
	gotoxy(columnForPlayer(player) + namelength - 2, rowForDataRow(row));
	cputs("  ");
	if (value == 0 && temp)
	{
		return;
	}
	if (value < 0)
	{
		return;
	}
	gotoxy(columnForPlayer(player) + namelength - strlen(numbuf), rowForDataRow(row));
	cputs(numbuf);
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
	return FALSE;
}

unsigned char currentDiceSum()
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

char checkStreet()
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

	for (row = 0; row < 6; row++)
	{
		if (ktable[row][currentPlayer] == -1)
		{
			displayTableEntry(currentPlayer, row, tvals[row], 1);
		}
	}

	for (row = 9; row < 16; row++)
	{
		if (ktable[row][currentPlayer] == -1)
		{
			displayTableEntry(currentPlayer, row, tvals[row], 1);
		}
	}
}

void removeTvalDisplay()
{
	char row;
	for (row = 0; row < 6; ++row)
	{
		if (ktable[row][currentPlayer] == -1)
		{
			displayTableEntry(currentPlayer, row, 0, 1);
		}
	}
	for (row = 9; row < 16; ++row)
	{
		if (ktable[row][currentPlayer] == -1)
		{
			displayTableEntry(currentPlayer, row, 0, 1);
		}
	}
}

void updatePlayer(unsigned char row)
{
	unsigned char i, j, t, centerS;
	for (i = 0; i < numPlayers; i++)
	{
		textcolor(colLegend);
		gotoxy(columnForPlayer(i), row);
		j = strlen(pnames[i]);
		if (j > namelength)
			j = namelength;
		if (namelength - j >= 2)
		{
			for (centerS = 0; centerS < ((namelength - j) / 2); centerS++)
			{
				cputc(' ');
			}
		}
		if (i == currentPlayer && row == 0)
			revers(1);
		for (t = 0; t < j; t++)
		{
			cputc(pnames[i][t]);
		}
		revers(0);
	}
}

void displayBoard()
{
	unsigned char i;
	unsigned char xmax;
	unsigned char lineCol;
	xmax = 11 + (numPlayers * (namelength + 1));
	clrscr();
	textcolor(colTable);
	// horizontal lines
	gotoxy(0, 1);
	chline(xmax);
	gotoxy(0, 8);
	chline(xmax);
	gotoxy(0, 12);
	chline(xmax);
	gotoxy(0, 20);
	chline(xmax);
	gotoxy(0, 23);
	chline(xmax);
	// player columns
	for (i = 0; i < numPlayers; i++)
	{
		lineCol = columnForPlayer(i) - 1;
		gotoxy(lineCol, 0);
		cvline(23);
		gotoxy(lineCol, 1);
		cputc(123);
		gotoxy(lineCol, 8);
		cputc(123);
		gotoxy(lineCol, 12);
		cputc(123);
		gotoxy(lineCol, 20);
		cputc(123);
		gotoxy(lineCol, 23);
		cputc(241);
	}
	gotoxy(xmax, 0);
	cvline(23);
	gotoxy(xmax, 1);
	cputc(179);
	gotoxy(xmax, 8);
	cputc(179);
	gotoxy(xmax, 12);
	cputc(179);
	gotoxy(xmax, 20);
	cputc(179);
	gotoxy(xmax, 23);
	cputc(253);

	updatePlayer(0);
	// rows
	for (i = 0; i < 18; i++)
	{
		gotoxy(0, rowForDataRow(i));
		textcolor(colLegend);
		if (i < 6)
		{
			revers(1);
			cputc('a' + i);
			revers(0);
			cputc(' ');
		}
		else if (i > 8 && i < 16)
		{
			revers(1);
			cputc('g' + (i - 9));
			revers(0);
			cputc(' ');
		}
		else
		{
			cprintf("  ");
		}
		textcolor(textcolorForRow(i));
		cprintf(rownames[i]);
	}
	initDiceDisplay();
}

void gamePreflight()
{

	unsigned char i, j;

	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = TRUE;
		rollDice(i);
	}
	for (i = 0; i < 18; ++i)
	{
		for (j = 0; j < 4; ++j)
		{
			ktable[i][j] = -1;
		}
	}
}

void startgame()
{

	char i;

	clrscr();
	revers(1);
	textcolor(colSplash);
	cputc(176);
	for (i = 0; i < 38; ++i)
		cputc(192);
	cputc(174);
	cputc(221);
	textcolor(colSplashRed);
	cprintf("k");
	textcolor(colSplash);
	cprintf("kniffel v2.2                         ");
	cputc(221);
	cputc(221);
	cprintf("written by herr k. @ k-burg, 2019     ");
	cputc(221);
	cputc(221);
	cprintf("                                      ");
	cputc(221);
	cputc(221);
	cprintf("with special thanks to frau k.,       ");
	cputc(221);
	cputc(221);
	cprintf("buba k., candor k. and the 7 turtles! ");
	cputc(221);
	cputc(173);
	for (i = 0; i < 38; ++i)
		cputc(192);
	cputc(189);
	revers(0);
	clearLower();
	do
	{
		gotoxy(0, 9);
		cclear(39);
		gotoxy(0, 9);
		textcolor(colText);
		cprintf("wieviele mitspieler (2-4)? ");
		input(inbuf);
		numPlayers = atoi(inbuf);
		if (numPlayers == 0)
		{
			numPlayers = 4;
			cprintf("-> 4");
		}
	} while (numPlayers < 2 || numPlayers > 4);

	namelength = (21 / numPlayers) - 1;

	for (i = 0; i < numPlayers; i++)
	{
		gotoxy(0, 11 + i);
		do
		{
			cprintf("spieler %d name: ", i + 1);
			input(inbuf);
			if (strlen(inbuf) == 0)
			{
				if (i == 0)
				{
					cprintf("-> katja");
					strcpy(inbuf, "katja");
				}
				if (i == 1)
				{
					cprintf("-> stephan");
					strcpy(inbuf, "stephan");
				}
				if (i == 2)
				{
					cprintf("-> buba");
					strcpy(inbuf, "buba");
				}
				if (i == 3)
				{
					cprintf("-> schnitzel");
					strcpy(inbuf, "schnitzel");
				}
			}
		} while (strlen(inbuf) == 0);
		strcpy(pnames[i], inbuf);
	}
}

char hasChosenRerollDice()
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

void commitSort()
{
	unsigned char i;
	qsort(dvalues, 5, sizeof(unsigned char), dcompare);
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = FALSE;
	}
	showCurrentRoll();
}

char shouldCommitRow(unsigned char row)
{
	char jn;
	if (tvals[row] > 0)
	{
		return TRUE;
	}
	clearLower();
	centerLower("wirklich? null punkte?!");
	jn = cgetc();
	clearLower();
	return (jn=='j');
}

void commitRow(unsigned char row)
{
	ktable[row][currentPlayer] = tvals[row];
	displayTableEntry(currentPlayer, row, ktable[row][currentPlayer], 0);
}

void updateSums()
{
	unsigned char i;
	int upperSum;
	int lowerSum;
	int current;

	upperSum = 0;
	lowerSum = 0;

	for (i = 0; i < 6; ++i)
	{
		current = ktable[i][currentPlayer];
		if (current >= 0)
		{
			upperSum += current;
		}
	}

	ktable[6][currentPlayer] = upperSum;

	if (upperSum >= 63)
	{
		ktable[7][currentPlayer] = 35;
		ktable[8][currentPlayer] = upperSum + 35;
	}
	else
	{
		ktable[7][currentPlayer] = -2;
		ktable[8][currentPlayer] = upperSum;
	}

	for (i = 9; i < 16; i++)
	{
		current = ktable[i][currentPlayer];
		if (current >= 0)
		{
			lowerSum += current;
		}
	}

	ktable[16][currentPlayer] = lowerSum;
	ktable[17][currentPlayer] = lowerSum + ktable[8][currentPlayer];

	displayTableEntry(currentPlayer, 6, ktable[6][currentPlayer], 0);
	displayTableEntry(currentPlayer, 7, ktable[7][currentPlayer], 0);
	displayTableEntry(currentPlayer, 8, ktable[8][currentPlayer], 0);
	displayTableEntry(currentPlayer, 16, ktable[16][currentPlayer], 0);
	displayTableEntry(currentPlayer, 17, ktable[17][currentPlayer], 0);
}

void nextPlayer()
{
	unsigned char i;
	for (i = 0; i < 5; i++)
	{
		shouldRoll[i] = TRUE;
	}
	++currentPlayer;
	if (currentPlayer >= numPlayers)
	{
		currentPlayer = 0;
	}
	updatePlayer(0);
	eraseDice();
	centerLower("<return> = wuerfeln");
	gotoxy(0, 0);
	cprintf("     ");
	waitkey('\n');
	doTurnRoll();
}

void checkQuit()
{
	unsigned char i;
	unsigned char j;
	unsigned char unfinishedEntries;
	unfinishedEntries = 0;
	for (j = 0; j < numPlayers; j++)
	{
		for (i = 0; i < 18; i++)
		{
			if (ktable[i][j] == -1)
			{
				++unfinishedEntries;
			}
		}
	}
	quit = (unfinishedEntries == 0);
}

void postRound()
{
	unsigned char i;
	unsigned char j;
	char jn;
	clrscr();
	cprintf("*** runde zuende! ***");
	updatePlayer(3);
	for (j = 0; j < numPlayers; j++)
	{
		roundResults[currentRound][j] = ktable[17][j];
		totals[j] = 0;
	}
	for (i = 0; i <= currentRound; i++)
	{
		gotoxy(0, 5 + i);
		cprintf("runde %d", i + 1);
		for (j = 0; j < numPlayers; j++)
		{
			gotoxy(columnForPlayer(j), 5 + i);
			cprintf("%d", roundResults[i][j]);
			totals[j] = totals[j] + roundResults[i][j];
		}
	}
	gotoxy(0, 7 + currentRound);
	cputs("gesamt");
	for (j = 0; j < numPlayers; j++)
	{
		gotoxy(columnForPlayer(j), 5 + currentRound + 2);
		cprintf("%d", totals[j]);
	}

	gotoxy(0, 5 + currentRound + 5);
	cputs("neue runde? (j/n)");
	clearbuf();
	jn = cgetc();
	if (jn != 'n')
	{
		quit = FALSE;
		currentRound++;
	}
	else
	{
		quit = TRUE;
	}
}

void mainloop()
{
	unsigned char rollCount;
	unsigned char cmd;
	unsigned char idx;
	currentRound = 0;
	quit = FALSE;
	startgame();
	do
	{
		gamePreflight();
		displayBoard();
		currentPlayer = numPlayers - 1;
		rollCount = 1;
		nextPlayer();
		do
		{
			gotoxy(0, 0);
			textcolor(colCurrentRollIdx);
			cprintf("(%d/%d)", rollCount, MAX_ROLL_COUNT);
			plotDiceLegend(rollCount < MAX_ROLL_COUNT);
			clearbuf();
			cmd = cgetc();
			if (cmd >= '1' && cmd <= '5' && rollCount < MAX_ROLL_COUNT)
			{
				idx = cmd - 49;
				shouldRoll[idx] = !shouldRoll[idx];
				plotDice(idx, dvalues[idx], shouldRoll[idx]);
			}
			if (cmd == '\n' && rollCount < MAX_ROLL_COUNT && hasChosenRerollDice())
			{
				doTurnRoll();
				rollCount++;
			}
			if (cmd == 'q')
				quit = TRUE;
			if ((cmd >= 'a' && cmd <= 'f') || (cmd >= 'g' && cmd <= 'm'))
			{
				idx = cmd - 'a';
				if (cmd >= 'g')
				{
					idx += 3;
				}
				if (ktable[idx][currentPlayer] == -1)
				{
					if (shouldCommitRow(idx))
					{
						removeTvalDisplay();
						commitRow(idx);
						updateSums();
						rollCount = 1;
						checkQuit();
						if (!quit)
						{
							nextPlayer();
						}
					}
				}
			}
			if (cmd == 's')
			{
				commitSort();
			}
		} while (!quit);
		postRound();
	} while (!quit);
}

void splash()
{
	startup();
	gotoxy(12 + 0, 11);
	cprintf("stephan   katja");
	textcolor(2);
	gotoxy(12 + 8, 11);
	cputc(211);
	initIO();
	clrscr();
}

int main()
{
	splash();
	mainloop();
	return 0;
}
