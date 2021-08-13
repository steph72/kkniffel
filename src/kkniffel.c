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
#include "congui.h"

#define MAX_ROLL_COUNT 3
#define MAX_ROUNDS 50

#define BOTTOMY 24
#define MID_X 40
#define DELETEKEY 20
#define RETURNKEY '\n'

void refreshTvalsDisplay(void);
void removeTvalDisplay(void); // remove tval display
void plotDiceLegend(unsigned char flag);

unsigned int gSeed;
unsigned int gSessionCount;

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
	cg_gotoxy(0, BOTTOMY);
	cg_puts("                                  ");
}

void centerLine(char line, char *msg)
{
	cg_gotoxy(MID_X - (strlen(msg) / 2), line);
	cg_puts(msg);
}

void centerLower(char *msg)
{
	clearLower();
	cg_gotoxy((g_xmax / 2) - (strlen(msg) / 2), BOTTOMY);
	cg_revers(1);
	cg_puts(msg);
	cg_revers(0);
	cg_gotoxy(0, 0);
}

void waitkey(char key)
{
	while (cg_getkey() != key)
		;
}

#define DICELEGEND_POS 73

void plotDiceLegend(unsigned char flag)
{
	unsigned char i;
	cg_textcolor(colLegend);
	cg_revers(flag);
	for (i = 0; i < 5; ++i)
	{
		cg_gotoxy(DICELEGEND_POS, (i * 5) + 2);
		if (flag)
		{
			cg_putc('1' + i);
		}
		else
		{
			cg_putc(' ');
		}
	}
	cg_revers(0);
}

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
	cg_revers(0);
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
		} while (cg_kbhit() == 0);
	}
	doSingleRoll();
	for (i = 0; i < 5; i++)
	{
		kc_setShouldRoll(i, false);
	}
	kc_recalcTVals();
	refreshTvalsDisplay();
	kc_newRoll();
}

void input(char *buf)
{
	char currentChar;
	char currentPos;
	currentPos = 0;
	buf[0] = 0;
	cg_cursor(1);
	do
	{
		currentChar = cg_getkey();
		if (currentChar != RETURNKEY && currentChar != DELETEKEY)
		{
			buf[currentPos] = currentChar;
			cg_putc(currentChar);
			currentPos++;
		}
		if (currentChar == DELETEKEY && currentPos > 0)
		{
			currentPos--;
			cg_gotoxy(cg_wherex() - 1, cg_wherey());
			cg_putc(' ');
			cg_gotoxy(cg_wherex() - 1, cg_wherey());
		}
	} while (currentChar != RETURNKEY);
	buf[currentPos] = 0;
	cg_cursor(0);
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
	return (16 + (p * (namelength + 1)));
}

void displayTableEntry(char player, char row, int value, char temp)
{
	char color;

	if (temp)
	{
		color = colTempValue;
	}
	else
	{
		color = textcolorForRow(row);
	}
	cg_textcolor(color);
	itoa(value, numbuf, 10);
	cg_gotoxy(columnForPlayer(player) + namelength - 2, kc_rowForDataRow(row));
	cg_puts("  ");
	if (value == 0 && temp)
	{
		return;
	}
	if (value < 0)
	{
		return;
	}
	cg_gotoxy(columnForPlayer(player) + namelength - strlen(numbuf), kc_rowForDataRow(row));
	cg_puts(numbuf);
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
		cg_textcolor(colLegend);
		cg_gotoxy(columnForPlayer(i), row);
		j = strlen(_pname[i]);
		if (j > namelength)
			j = namelength;
		if (namelength - j >= 2)
		{
			for (centerS = 0; centerS < ((namelength - j) / 2); centerS++)
			{
				cg_putc(' ');
			}
		}
		if (i == _currentPlayer && row == 0)
			cg_revers(1);
		for (t = 0; t < j; t++)
		{
			cg_putc(_pname[i][t]);
		}
		cg_revers(0);
	}
}

void displayBoard()
{
	unsigned char i;
	unsigned char lineCol;

	if (benchmarkMode)
		return;

	g_xmax = 16 + (numPlayers * (namelength + 1));

	cg_clrscr();
	cg_textcolor(colTable);
	// horizontal lines
	/*
	hline(0,1,40,HLINE_STYLE_MID);
	hline(0,8,40,HLINE_STYLE_MID);
	hline(0,12,40,HLINE_STYLE_MID);
	hline(0,20,40,HLINE_STYLE_MID);
	hline(0,23,40,HLINE_STYLE_MID);
	*/
	// player columns
	for (i = 0; i < numPlayers; i++)
	{
		lineCol = columnForPlayer(i) - 1;
		cg_gotoxy(lineCol, 0);
		// cvline(23);
		cg_gotoxy(lineCol, 1);
		cg_putc(123);
		cg_gotoxy(lineCol, 8);
		cg_putc(123);
		cg_gotoxy(lineCol, 12);
		cg_putc(123);
		cg_gotoxy(lineCol, 20);
		cg_putc(123);
		cg_gotoxy(lineCol, 23);
		cg_putc(241);
	}
	cg_gotoxy(g_xmax, 0);
	// cvline(23);
	cg_gotoxy(g_xmax, 1);
	cg_putc(179);
	cg_gotoxy(g_xmax, 8);
	cg_putc(179);
	cg_gotoxy(g_xmax, 12);
	cg_putc(179);
	cg_gotoxy(g_xmax, 20);
	cg_putc(179);
	cg_gotoxy(g_xmax, 23);
	cg_putc(253);
	updatePlayer(0);
	// rows
	for (i = 0; i < 18; i++)
	{
		cg_gotoxy(0, kc_rowForDataRow(i));
		cg_textcolor(colLegend);
		if (kc_letterForRow(i))
		{
			cg_revers(1);
			cg_putc(kc_letterForRow(i));
			cg_revers(0);
			cg_putc(' ');
		}
		else
		{
			cg_puts("  ");
		}
		cg_textcolor(textcolorForRow(i));
		cg_printf(kc_labelForRow(i));
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
	cg_clrscr();
	cg_puts("** benchmark mode **");
	cg_gotoxy(0, 3);
	cg_puts("press alphanumeric key for\r\nrandom seed: ");
	cg_cursor(1);
	gSeed = cg_getkey() * 555;
	cg_printf("-> %u\r\n", seed);
	cg_cursor(0);
	cg_puts("\r\nrunning kkniffelbench...");
}

void bannerDice()
{
	char i;
	cg_textcolor(colDice);

	cg_clrscr();
	for (i = 0; i < 8; ++i)
	{
		_plotDice(1 + (i % 6), i * 5, 0, 0);
		_plotDice(1 + (i % 6), i * 5, BOTTOMY - 4, 0);
	}
}

void displayCredits()
{
	bannerDice();
	cg_textcolor(colText);
	cg_revers(1);
	centerLine(5, " * k k n i f f e l * ");
	cg_revers(0);
	centerLine(7, "written by stephan kleinert");
	centerLine(8, "at k-burg, bad honnef,");
	centerLine(9, "hundehaus im reinhardswald and");
	centerLine(10, "at k cottage, erl");
	centerLine(11, "2019/20");
	centerLine(13, "with very special thanks to");
	centerLine(14, "frau k., buba k. candor k.");
	centerLine(15, "and the seven turtles!");
	centerLine(17, "-- key --");
	cg_getkey();
}

void displayInstructions()
{
	bannerDice();
	cg_textcolor(colText);
	cg_revers(1);
	centerLine(5, " * instructions * ");
	cg_revers(0);
	centerLine(7, "game keys:");
	centerLine(9, "<return> to roll or reroll the dice");
	centerLine(11, "<1-6> to select dice to reroll");
	centerLine(12, "<a-m> to choose category to score");
	centerLine(14, "<s> to sort the dice");
	centerLine(16, "-- key --");
	cg_getkey();
}

void showHighscores(char *title, unsigned char positions[], unsigned char save)
{
	int i;
	int j;
	int pos;
	cg_clrscr();
	cg_textcolor(colBonus);
	centerLine(0, title);
	cg_textcolor(colText);
	for (i = 1; i <= HS_LISTSIZE; ++i)
	{
		centerLine(1 + (2 * i), highscoreAtPos(i));
	}
	if (positions)
	{

		cg_textcolor(colSplashRed);
		for (j = 0; j < numPlayers; ++j)
		{
			pos = positions[j];
			if (pos)
			{
				centerLine(1 + (2 * pos), highscoreAtPos(pos));
			}
		}
	}

	cg_textcolor(colLowerSum);
	if (save)
	{
		centerLine(24, "please wait, saving...");
		saveHighscores();
	}
	centerLine(24, "   - press any key -  ");
	cg_getkey();
}

void startgame()
{
	char i;
	char c;
	char *strbuf;
	char promptTopRow;
	char sessionCountRow[40];

	sprintf(sessionCountRow, "k-cottage session #%d", gSessionCount);
	promptTopRow = (BOTTOMY / 2) - 5;

	do
	{
		bannerDice();
		cg_textcolor(colText);
		cg_revers(1);
#ifdef DEBUG
		gotoxy(0, 0);
		cputs("** debug build! **");
#endif
		centerLine(promptTopRow, (char *)gTitle);
		cg_revers(0);
		centerLine(promptTopRow + 2, "- version 2.4 -");
		centerLine(promptTopRow + 3, "written by stephan kleinert");
		cg_textcolor(colBonus);
		centerLine(promptTopRow + 10, "or i)instructions c)redits h)ighscores");
		cg_textcolor(colLowerSum);
		centerLine(promptTopRow + 6, sessionCountRow);
		centerLine(promptTopRow + 7, "how many players (2-4)?");

		cg_cursor(1);
		c = tolower(cg_getkey());
		numPlayers = c - '0';
		cg_cursor(0);
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

	cg_clrscr();

#define TABLE_WIDTH 50

	namelength = (TABLE_WIDTH / numPlayers) - 1;
	namelength = (TABLE_WIDTH / numPlayers) - 1;

	cg_putsxy(0, 20, "(add '@' to player name to\r\ncreate a computer player!)");

	for (i = 0; i < numPlayers; i++)
	{
		cg_gotoxy(0, 12 + i);
		//do
		//{
			cg_printf("player %d name: ", i + 1);
			strbuf=cg_input(16);
			if (strlen(strbuf) == 0)
			{
				if (i == 0)
				{
					cg_printf("-> katja @");
					strcpy(strbuf, "katja @");
				}
				if (i == 1)
				{
					cg_printf("-> stephan @");
					strcpy(strbuf, "stephan @");
				}
				if (i == 2)
				{
					cg_printf("-> buba @");
					strcpy(strbuf, "buba @");
				}
				if (i == 3)
				{
					cg_printf("-> schnitzel @");
					strcpy(strbuf, "schnitzel @");
				}
			}
		// } while (strlen(strbuf) == 0);
		if (strchr(strbuf, '@'))
		{
			cg_puts(" (cp) ");
			kc_setIsComputerPlayer(i, true);
		}
		strcpy(_pname[i], strbuf);
		free(strbuf);
	}

	cg_cursor(0);
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
	jn = tolower(cg_getkey());
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
	cg_gotoxy(0, 0);
	cg_puts("          ");
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
			cg_revers(rOn);
			displayTableEntry(_currentPlayer, row, kc_tableValue(row, _currentPlayer, 0), 0);
			jiffySleep(5);
		}
		cg_revers(0);
	}
	else
	{
		cg_gotoxy(28, 0);
		cg_printf("[%d/%d]", ++benchmarkRolls, benchmarkRollsToDo);
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

	cg_puts("FOOOOOO!");

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

	cg_textcolor(colText);
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

	cg_clrscr();
	cg_printf("*** round finished! ***\r\n\r\n");

	resultsTop = cg_wherey() + 4;
	updatePlayer(cg_wherey() + 2);

	for (i = 0; i <= currentRound; i++)
	{
		cg_gotoxy(0, resultsTop + i);
		cg_printf("round %d", i + 1);
		for (j = 0; j < numPlayers; j++)
		{
			res = roundResults[i][j];
			if (res > 250)
			{
				cg_textcolor(colLowerSum);
			}
			cg_gotoxy(columnForPlayer(j), resultsTop + i);
			cg_printf("%d", res);
			totals[j] = totals[j] + res;
			cg_textcolor(colText);
		}
	}
	cg_gotoxy(0, resultsTop + 2 + currentRound);

	if (benchmarkMode)
	{
		cg_gotoxy(0, 0);
		cg_printf("\r\n#%d avg %d     ", numResults, statTotal / numResults);
	}
	else
	{
		cg_puts("total");
		for (j = 0; j < numPlayers; j++)
		{
			cg_gotoxy(columnForPlayer(j), resultsTop + currentRound + 2);
			cg_printf("%d", totals[j]);
		}
	}

	cg_gotoxy(0, resultsTop + currentRound + 5);

	if (!benchmarkMode)
	{
		cg_puts("another game? (y/n)");
		jn = tolower(cg_getkey());
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
		cg_gotoxy(0, 0);
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
	// debugDumpChoices();
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
				cg_gotoxy(0, 0);
				cg_textcolor(colCurrentRollIdx);
				cg_printf("(roll %d/%d)", kc_getRollCount(), MAX_ROLL_COUNT);
				plotDiceLegend(kc_getRollCount() < MAX_ROLL_COUNT);
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

				cmd = tolower(cg_getkey());
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
					// debugDumpChoices();
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
	cg_init(true,false,NULL);

	gSeed = getJiffies();
	srand(gSeed);
	benchmarkMode = false;
	statTotal = 0;
	numResults = 0;
	cg_gotoxy(0, 0);

	cg_puts("stephan   katja");
	cg_textcolor(2);
	cg_gotoxy(8, 0);
	cg_putc(211);
	cg_gotoxy(0, 2);

	gSessionCount = kc_incrementAndGetSessionCount();
	cg_clrscr();
	initHighscores();
	cg_clrscr();
}

int main()
{
	initGame();
	mainloop();
	return 0;
}
