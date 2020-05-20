/*

	KKNIFFEL
	by Stephan Kleinert

	A yahtzee type game for various 8 bit computers... :)

	--------------------------------------------------------------------

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

#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <6502.h>
#include <time.h>

#include "io.h"
#include "kcore.h"
#include "cplayer.h"
#include "highscores.h"

#define MAX_ROLL_COUNT 3
#define MAX_ROUNDS 50

#pragma warn(no-effect, off)

#define BOTTOMY 24

#ifdef __APPLE2__
#undef BOTTOMY
#define BOTTOMY 23
#endif

#ifdef __CX16__
#undef BOTTOMY
#define BOTTOMY 29
#endif

#ifdef __APPLE2__
#define DELETEKEY 8
#define RETURNKEY 13
#else
#define DELETEKEY 20
#define RETURNKEY '\n'
#endif

void refreshTvalsDisplay(void);
void removeTvalDisplay(void); // remove tval display
void plotDiceLegend(unsigned char flag);

unsigned int gSeed;
unsigned int gSessionCount;

char inbuf[40];

int roundResults[MAX_ROUNDS][4]; // results for postgame
int totals[4];					 // totals per player for postgame

unsigned char quit;
unsigned char currentRound;

char numPlayers;
char namelength;

char benchmarkMode;

unsigned int statTotal;
int benchmarkRolls;
int benchmarkRollsToDo;
char numResults;

unsigned char g_xmax;

void jiffySleep(int num);

#ifndef __APPLE2__
unsigned int getJiffies()
{
	return clock();
}
#endif

void clearLower(void)
{
	gotoxy(0, BOTTOMY);
#ifdef __APPLE2__
	chline(g_xmax);
#else
	cputs("                                  ");
#endif
}

void centerLine(char line, char *msg)
{
	gotoxy(20 - (strlen(msg) / 2), line);
	cputs(msg);
}

void centerLower(char *msg)
{
	clearLower();
	gotoxy((g_xmax / 2) - (strlen(msg) / 2), BOTTOMY);
#ifdef __APPLE2__
	revers(1);
#endif
	cputs(msg);
#ifdef __APPLE2__
	revers(0);
#endif
	gotoxy(0, 0);
}

void clearbuf(void)
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

#ifndef __APPLE2__
#define DICELEGEND_POS 34
#else
#define DICELEGEND_POS 33
#endif

#ifndef __CX16__
void plotDiceLegend(unsigned char flag)
{
	unsigned char i;
	textcolor(colLegend);
	revers(flag);
	for (i = 0; i < 5; ++i)
	{
		gotoxy(DICELEGEND_POS, (i * 5) + 2);
		if (flag)
		{
			cputc('1' + i);
		}
		else
		{
			cputc(' ');
		}
	}

	/*  we need to place the cursor on an empty space
	    after marking the dice keys in reverse
	    due to a strange bug in the plus/4 conio, where 
	    stopping reverse mode also resets the colour
	    under the cursor...                             */

	gotoxy(33, (i * 5) + 2);
	revers(0);
}
#endif

void doSingleRoll()
{
	unsigned char i;
	kc_doSingleRoll();
	if (benchmarkMode)
		return;
	for (i = 0; i < 5; ++i)
	{
		if (kc_getShouldRoll(i))
		{
			plotDice(i, kc_diceValue(i), false);
		}
	}
}

void showCurrentRoll()
{
	unsigned char i;

	if (benchmarkMode)
		return;

	for (i = 0; i < 5; i++)
	{
		plotDice(i, kc_diceValue(i), kc_getShouldRoll(i));
	}
}

void eraseDice()
{
	unsigned char i;
	revers(0);
	for (i = 0; i < 5; i++)
	{
		eraseDie(i);
	}
}

void doTurnRoll()
{
	char i;
	unsigned int j;
	clearLower();
	removeTvalDisplay();
	showCurrentRoll();
	clearbuf();
	if (kc_getIsComputerPlayer(_currentPlayer))
	{
		if (!benchmarkMode)

#ifndef __APPLE2__
			j = getJiffies();
		do
		{
#endif

			for (i = 0; i < 20; ++i)
			{
				doSingleRoll();
			}

#ifndef __APPLE2__
		} while (getJiffies() - j < 60);
#endif
	}
	else
	{
		centerLower("<return> = stop rolling");
		do
		{
			doSingleRoll();
		} while (kbhit() == 0);
	}
	doSingleRoll();
	for (i = 0; i < 5; i++)
	{
		kc_setShouldRoll(i, false);
	}
	kc_recalcTVals();
	refreshTvalsDisplay();
	kc_newRoll();
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
		if (currentChar != RETURNKEY && currentChar != DELETEKEY)
		{
			buf[currentPos] = currentChar;
			cputc(currentChar);
			currentPos++;
		}
		if (currentChar == DELETEKEY && currentPos > 0)
		{
			currentPos--;
			gotox(wherex() - 1);
			cputc(' ');
			gotox(wherex() - 1);
		}
	} while (currentChar != RETURNKEY);
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

void displayTableEntry(char player, char row, int value, char temp)
{
	char color;

	if (temp)
	{
#if defined(__PET__) || defined(__APPLE2__)
		revers(value > 0);
#else
		color = colTempValue;
#endif
	}
	else
	{
		color = textcolorForRow(row);
	}
	textcolor(color);
	itoa(value, numbuf, 10);
	gotoxy(columnForPlayer(player) + namelength - 2, kc_rowForDataRow(row));
	cputs("  ");
	if (value == 0 && temp)
	{
		return;
	}
	if (value < 0)
	{
		return;
	}
	gotoxy(columnForPlayer(player) + namelength - strlen(numbuf), kc_rowForDataRow(row));
	cputs(numbuf);
}

void refreshTvalsDisplay(void)
{
	unsigned char row;

	if (benchmarkMode)
		return;

	for (row = 0; row < 6; row++)
	{
		if (kc_tableValue(row, _currentPlayer, 0) == -1)
		{
			displayTableEntry(_currentPlayer, row, tvals[row], 1);
		}
	}

	for (row = 9; row < 16; row++)
	{
		if (kc_tableValue(row, _currentPlayer, 0) == -1)
		{
			displayTableEntry(_currentPlayer, row, tvals[row], 1);
		}
	}
}

void removeTvalDisplay()
{
	char row;

	if (benchmarkMode)
		return;

	for (row = 0; row < 6; ++row)
	{
		if (kc_tableValue(row, _currentPlayer, 0) == -1)
		{
			displayTableEntry(_currentPlayer, row, 0, 1);
		}
	}
	for (row = 9; row < 16; ++row)
	{
		if (kc_tableValue(row, _currentPlayer, 0) == -1)
		{
			displayTableEntry(_currentPlayer, row, 0, 1);
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
		j = strlen(_pname[i]);
		if (j > namelength)
			j = namelength;
		if (namelength - j >= 2)
		{
			for (centerS = 0; centerS < ((namelength - j) / 2); centerS++)
			{
				cputc(' ');
			}
		}
		if (i == _currentPlayer && row == 0)
			revers(1);
		for (t = 0; t < j; t++)
		{
			cputc(_pname[i][t]);
		}
		revers(0);
	}
}

void displayBoard()
{
	unsigned char i;
	unsigned char lineCol;

	if (benchmarkMode)
		return;

	g_xmax = 11 + (numPlayers * (namelength + 1));

	clrscr();
	textcolor(colTable);
	// horizontal lines
	gotoxy(0, 1);
	chline(g_xmax);
	gotoxy(0, 8);
	chline(g_xmax);
	gotoxy(0, 12);
	chline(g_xmax);
	gotoxy(0, 20);
	chline(g_xmax);
	gotoxy(0, 23);
	chline(g_xmax);
	// player columns
	for (i = 0; i < numPlayers; i++)
	{
		lineCol = columnForPlayer(i) - 1;
		gotoxy(lineCol, 0);
		cvline(23);
#ifndef __APPLE2__
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
#endif
	}
	gotoxy(g_xmax, 0);
	cvline(23);
#ifndef __APPLE2__
	gotoxy(g_xmax, 1);
	cputc(179);
	gotoxy(g_xmax, 8);
	cputc(179);
	gotoxy(g_xmax, 12);
	cputc(179);
	gotoxy(g_xmax, 20);
	cputc(179);
	gotoxy(g_xmax, 23);
	cputc(253);
#endif

	updatePlayer(0);
	// rows
	for (i = 0; i < 18; i++)
	{
		gotoxy(0, kc_rowForDataRow(i));
		textcolor(colLegend);
		if (kc_letterForRow(i))
		{
			revers(1);
			cputc(kc_letterForRow(i));
			revers(0);
			cputc(' ');
		}
		else
		{
			cputs("  ");
		}
		textcolor(textcolorForRow(i));
		cprintf(kc_labelForRow(i));
	}
	initDiceDisplay();
}

void startBenchmarkMode()
{
	char i;
	unsigned int seed;
	benchmarkMode = true;
	numPlayers = 4;
	namelength = 4;
	benchmarkRolls = 0;
	benchmarkRollsToDo = (4 * 13) * MAX_ROUNDS;
	for (i = 0; i < 4; ++i)
	{
		kc_setIsComputerPlayer(i, true);
		strcpy(_pname[i], "cp");
	}
	clrscr();
	cputs("** benchmark mode **");
	gotoxy(0, 3);
	cputs("press alphanumeric key for\r\nrandom seed: ");
	cursor(1);
	gSeed = cgetc() * 555;
	cprintf("-> %u\r\n", seed);
	cursor(0);
	cputs("\r\nrunning kkniffelbench...");
}

void bannerDice()
{
	char i;
	textcolor(colDice);

	clrscr();
	for (i = 0; i < 8; ++i)
	{
		_plotDice(1 + (rand() % 6), i * 5, 0, 0);
		_plotDice(1 + (rand() % 6), i * 5, BOTTOMY - 4, 0);
	}
}

void displayCredits()
{
	bannerDice();
	textcolor(colText);
	revers(1);
	centerLine(5, " * k k n i f f e l * ");
	revers(0);
	centerLine(7, "written by stephan kleinert");
	centerLine(8, "at k-burg, bad honnef,");
	centerLine(9, "hundehaus im reinhardswald and");
	centerLine(10, "at k cottage, erl");
	centerLine(11, "2019/20");
	centerLine(13, "with very special thanks to");
	centerLine(14, "frau k., buba k. candor k.");
	centerLine(15, "and the seven turtles!");
	centerLine(17, "-- key --");
	cgetc();
}

void displayInstructions()
{
	bannerDice();
	textcolor(colText);
	revers(1);
	centerLine(5, " * instructions * ");
	revers(0);
	centerLine(7, "game keys:");
	centerLine(9, "<return> to roll or reroll the dice");
	centerLine(11, "<1-6> to select dice to reroll");
	centerLine(12, "<a-m> to choose category to score");
	centerLine(14, "<s> to sort the dice");
	centerLine(16, "-- key --");
	cgetc();
}

void showHighscores(char *title, unsigned char positions[], unsigned char save)
{
	int i;
	int j;
	int pos;
	clrscr();
	textcolor(colBonus);
	centerLine(0, title);
	textcolor(colText);
	for (i = 1; i <= HS_LISTSIZE; ++i)
	{
		centerLine(1 + (2 * i), highscoreAtPos(i));
	}
	if (positions)
	{
#if defined(__PET__) || defined(__APPLE2__)
		revers(1);
#endif
		textcolor(colSplashRed);
		for (j = 0; j < numPlayers; ++j)
		{
			pos = positions[j];
			if (pos)
			{
				centerLine(1 + (2 * pos), highscoreAtPos(pos));
			}
		}
	}
#if defined(__PET__) || defined(__APPLE2__)
	revers(0);
#endif
	textcolor(colLowerSum);
	if (save)
	{
		centerLine(24, "please wait, saving...");
		saveHighscores();
	}
	centerLine(24, "   - press any key -  ");
	clearbuf();
	cgetc();
}

void startgame()
{
	char i;
	char c;
	char promptTopRow;
	char sessionCountRow[40];

	sprintf(sessionCountRow, "k-cottage session #%d", gSessionCount);
	promptTopRow = (BOTTOMY / 2) - 5;

	do
	{
		bannerDice();
		textcolor(colText);
		revers(1);
#ifdef DEBUG
		gotoxy(0, 0);
		cputs("** debug build! **");
#endif
		centerLine(promptTopRow, (char *)gTitle);
		revers(0);
		centerLine(promptTopRow + 2, "- version 2.4 -");
		centerLine(promptTopRow + 3, "written by stephan kleinert");
		textcolor(colBonus);
		centerLine(promptTopRow + 10, "or i)instructions c)redits h)ighscores");
		textcolor(colLowerSum);
		centerLine(promptTopRow + 6, sessionCountRow);
		centerLine(promptTopRow + 7, "how many players (2-4)?");

		cursor(1);
		c = tolower(cgetc());
		numPlayers = c - '0';
		cursor(0);
		if (c == 'i')
		{
			displayInstructions();
		}
		else if (c == 'c')
		{
			displayCredits();
		}
		else if (c == '#')
		{
			startBenchmarkMode();
			return;
		}
		else if (c == 'h')
		{
			showHighscores("** high scores **", NULL, false);
		}
	} while (numPlayers < 2 || numPlayers > 4);

	clrscr();

#ifdef __CX16__
#define TABLE_WIDTH 27
#else
#define TABLE_WIDTH 21
#endif

	namelength = (TABLE_WIDTH / numPlayers) - 1;
	namelength = (TABLE_WIDTH / numPlayers) - 1;

	cputsxy(0, 20, "(add '@' to player name to\r\ncreate a computer player!)");

	for (i = 0; i < numPlayers; i++)
	{
		gotoxy(0, 12 + i);
		do
		{
			cprintf("player %d name: ", i + 1);
			input(inbuf);
			if (strlen(inbuf) == 0)
			{
				if (i == 0)
				{
					cprintf("-> katja @");
					strcpy(inbuf, "katja @");
				}
				if (i == 1)
				{
					cprintf("-> stephan @");
					strcpy(inbuf, "stephan @");
				}
				if (i == 2)
				{
					cprintf("-> buba @");
					strcpy(inbuf, "buba @");
				}
				if (i == 3)
				{
					cprintf("-> schnitzel @");
					strcpy(inbuf, "schnitzel @");
				}
			}
		} while (strlen(inbuf) == 0);
		if (strchr(inbuf, '@'))
		{
			cputs(" (cp) ");
			kc_setIsComputerPlayer(i, true);
		}
		strcpy(_pname[i], inbuf);
	}

	cursor(0);
}

char shouldCommitRow(unsigned char row)
{
	char jn;
	if (tvals[row] > 0)
	{
		return true;
	}
	clearLower();
	centerLower("really? zero points (y/n)?!");
	jn = tolower(cgetc());
	clearLower();
	return (jn == 'y');
}

void updateSumDisplay()
{
	displayTableEntry(_currentPlayer, 6, kc_tableValue(6, _currentPlayer, 0), 0);
	displayTableEntry(_currentPlayer, 7, kc_tableValue(7, _currentPlayer, 0), 0);
	displayTableEntry(_currentPlayer, 8, kc_tableValue(8, _currentPlayer, 0), 0);
	displayTableEntry(_currentPlayer, 16, kc_tableValue(16, _currentPlayer, 0), 0);
	displayTableEntry(_currentPlayer, 17, kc_tableValue(17, _currentPlayer, 0), 0);
}

void doNextPlayer()
{
	kc_newTurn();
	if (benchmarkMode)
		return;

	updatePlayer(0);
	eraseDice();
	gotoxy(0, 0);
	cputs("          ");
	if (!kc_getIsComputerPlayer(_currentPlayer))
	{
		centerLower("<return> = start rolling");
		waitkey(RETURNKEY);
	}
	doTurnRoll();
}

void checkQuit()
{
	quit = kc_checkQuit();
}

void commitRow(unsigned char row)
{
	unsigned char i;
	unsigned char rOn;

	if (!kc_getIsComputerPlayer(_currentPlayer))
	{
		if (!shouldCommitRow(row))
		{
			return;
		}
	}

	rOn = 0;

	kc_commitRow(row);

	if (!benchmarkMode)
	{
		removeTvalDisplay();
		updateSumDisplay();
		for (i = 0; i < 6; ++i)
		{
			rOn = !rOn;
			revers(rOn);
			displayTableEntry(_currentPlayer, row, kc_tableValue(row, _currentPlayer, 0), 0);
			jiffySleep(5);
		}
		revers(0);
	}
	else
	{
		gotoxy(28, 0);
		cprintf("[%d/%d]", ++benchmarkRolls, benchmarkRollsToDo);
	}

	checkQuit();

	if (!quit)
	{
		doNextPlayer();
	}
}

int pcomp(const void *a, const void *b)
{
	int resA;
	int resB;

	cputs("FOOOOOO!");

	resA = kc_tableValue(17, *(unsigned char *)a, 0);
	resB = kc_tableValue(17, *(unsigned char *)b, 0);

	if (resA == resB)
	{
		return 0;
	}
	else if (resA < resB)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void postRound()
{
	unsigned char newHighscorePositions[4];
	unsigned char newH;
	unsigned char i;
	unsigned char j;
	int res;
	unsigned char sortedPlayers[4];
	unsigned char pos;
	unsigned char resultsTop;
	char jn;

	newH = false;

	for (i = 0; i < numPlayers; i++)
	{
		sortedPlayers[i] = i;
		newHighscorePositions[i] = 0;
	}

	qsort(sortedPlayers, numPlayers, 1, pcomp);

	textcolor(colText);
	for (j = 0; j < numPlayers; j++)
	{
		i = sortedPlayers[j];

		res = kc_tableValue(17, i, 0);
		pos = checkAndCommitHighscore(res, _pname[i]);
		if (pos)
		{
			newH = true;
			newHighscorePositions[j] = pos;
		}

		roundResults[currentRound][i] = res;
		statTotal += res;
		numResults++;
		totals[i] = 0;
	}

	if (newH)
	{
		showHighscores("new highscores!", newHighscorePositions, true);
	}

	clrscr();
	cprintf("*** round finished! ***\r\n\r\n");

	resultsTop = wherey() + 4;
	updatePlayer(wherey() + 2);

	for (i = 0; i <= currentRound; i++)
	{
		gotoxy(0, resultsTop + i);
		cprintf("round %d", i + 1);
		for (j = 0; j < numPlayers; j++)
		{
			res = roundResults[i][j];
			if (res > 250)
			{
				textcolor(colLowerSum);
			}
			gotoxy(columnForPlayer(j), resultsTop + i);
			cprintf("%d", res);
			totals[j] = totals[j] + res;
			textcolor(colText);
		}
	}
	gotoxy(0, resultsTop + 2 + currentRound);

	if (benchmarkMode)
	{
		gotoxy(0, 0);
		cprintf("\r\n#%d avg %d     ", numResults, statTotal / numResults);
	}
	else
	{
		cputs("total");
		for (j = 0; j < numPlayers; j++)
		{
			gotoxy(columnForPlayer(j), resultsTop + currentRound + 2);
			cprintf("%d", totals[j]);
		}
	}

	gotoxy(0, resultsTop + currentRound + 5);

	if (!benchmarkMode)
	{
		cputs("another game? (y/n)");
		clearbuf();
		jn = tolower(cgetc());
		if (jn != 'n')
		{
			quit = false;
			currentRound++;
		}
		else
		{
			quit = true;
		}
	}
	else
	{
		if (currentRound < MAX_ROUNDS - 1)
		{
			currentRound++;
			quit = false;
		}
		else
		{
			quit = true;
		}
	}
}

#ifndef __APPLE2__
void jiffySleep(int num)
{
	unsigned int t;

	t = getJiffies();
	while ((getJiffies() - t) < num)
		;
}
#endif

void doCP()
{
	int exitVal = 0;

	if (!benchmarkMode)
	{
		centerLower("thinking...");
	}

	cp_analyze();

	if (!benchmarkMode)
	{
		jiffySleep(80);
		gotoxy(0, 0);
		clearLower();
	}

	if (kc_getRollCount() == 3)
	{
		exitVal = cp_exitRow();
		commitRow(exitVal);
	}
	else
	{
		exitVal = cp_markDice();
		showCurrentRoll();
		if (!benchmarkMode)
			jiffySleep(60);
		if (exitVal >= 0)
		{
			commitRow(exitVal);
			return;
		}
		else
		{
			doTurnRoll();
		}
	}
}

#ifdef DEBUG
void debugSetRoll()
{
	int i;
	char in;
	for (i = 0; i < 5; i++)
	{
		kc_setShouldRoll(i, false);
	}
	for (i = 0; i < 5; ++i)
	{
		gotoxy(0, 24);
		cprintf("give value for die %d ", i + 1);
		in = cgetc() - '0';
		if (in > 0 && in <= 6)
		{
			kc_setDiceValue(i, in);
		}
		showCurrentRoll();
	}
	gotoxy(0, 24);
	cputs("                      ");
	kc_recalcTVals();
	refreshTvalsDisplay();
	debugDumpChoices();
}
#endif

void mainloop()
{
	unsigned char cmd;
	unsigned char idx;
	currentRound = 0;
	quit = false;
	startgame();
	do
	{
		kc_initGame(numPlayers, 1);
		displayBoard();
		_currentPlayer = numPlayers; // doNextPlayer() goes to player 1
		doNextPlayer();
		do
		{
			if (!benchmarkMode)
			{
				gotoxy(0, 0);
				textcolor(colCurrentRollIdx);
				cprintf("(roll %d/%d)", kc_getRollCount(), MAX_ROLL_COUNT);
				plotDiceLegend(kc_getRollCount() < MAX_ROLL_COUNT);
				clearbuf();
			}

			if (kc_getIsComputerPlayer(_currentPlayer))
			{
				doCP();
			}
			else
			{

				if (kc_getRollCount() < 3)
				{
					centerLower("[a-m] or [1-5 + ret]");
				}
				else
				{
					centerLower("[a-m]");
				}

				cmd = tolower(cgetc());
#ifdef DEBUG
				if (cmd == 'q')
				{
					kc_debugFill(row_sixes);
				}
				if (cmd == '!')
				{
					doCP();
				}
				if (cmd == '?')
				{
					debugDumpChoices();
				}
				if (cmd == '#')
				{
					debugSetRoll();
				}
#endif

				if (cmd >= '1' && cmd <= '5' && kc_getRollCount() < 3)
				{
					idx = cmd - 49;
					kc_toggleShouldRoll(idx);
					plotDice(idx, kc_diceValue(idx), kc_getShouldRoll(idx));
				}
				if (cmd == ' ' && kc_getRollCount() < 3)
				{
					centerLower("katja-move!");
					for (idx = 0; idx < 5; ++idx)
					{
						kc_setShouldRoll(idx, true);
						plotDice(idx, kc_diceValue(idx), kc_getShouldRoll(idx));
					}
					clearLower();
				}
				if (cmd == RETURNKEY && kc_canRoll())
				{
					doTurnRoll();
				}

				if ((cmd >= 'a' && cmd <= 'f') || (cmd >= 'g' && cmd <= 'm'))
				{
					idx = cmd - 'a';
					if (cmd >= 'g')
					{
						idx += 3;
					}
					if (kc_tableValue(idx, _currentPlayer, 0) == -1)
					{
						commitRow(idx);
					}
				}
				if (cmd == 's')
				{
					kc_commitSort();
					showCurrentRoll();
				}
			}
		} while (!quit);
		postRound();
	} while (!quit);
}

void initGame()
{
	startup();

#ifndef __APPLE2__
	gSeed = getJiffies();
#endif

	srand(gSeed);
	benchmarkMode = false;
	statTotal = 0;
	numResults = 0;
	gotoxy(0, 0);

#ifdef __CBM__
	cputs("stephan   katja");
	textcolor(COLOR_RED);
	gotoxy(8, 0);
	cputc(211);
	gotoxy(0, 2);
#else
	cputs("stephan <3 katja");
#endif

	gSessionCount = kc_incrementAndGetSessionCount();
	clrscr();
	initHighscores();
	initIO();
	clrscr();
}

int main()
{
	initGame();
	mainloop();
	return 0;
}
