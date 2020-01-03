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
#include <stdlib.h>
#include <string.h>
#include <6502.h>
#include <time.h>

#include "io.h"
#include "kcore.h"
#include "cplayer.h"

#define MAX_ROLL_COUNT 3

// clang-format off
#if defined(__PET__)
#pragma warn(no-effect, off)
#endif
// clang-format on

char inbuf[40];

int roundResults[10][4]; // results for postgame
int totals[4];			 // totals per player for postgame

void refreshTvalsDisplay(void);
void removeTvalDisplay(void); // remove tval display

unsigned char quit;
unsigned char currentRound;

char numPlayers;
char namelength;

unsigned int getJiffies()
{
	return clock();
}

void clearLower(void)
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

	/*  we need to place the cursor on an empty space
	    after marking the dice keys in reverse
	    due to a strange bug in the plus/4 conio, where 
	    stopping reverse mode also resets the colour
	    under the cursor...                             */

	gotoxy(33, (i * 5) + 2);
	revers(0);
}

void doSingleRoll()
{
	unsigned char i;
	kc_doSingleRoll();
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
		gotoxy(34, (i * 5) + 2);
		cputc(' ');
	}
}

void doTurnRoll()
{
	char i;
	clearLower();
	removeTvalDisplay();
	showCurrentRoll();
	clearbuf();
	if (kc_getIsComputerPlayer(_currentPlayer))
	{
		for (i = 0; i < 20; ++i)
		{
			doSingleRoll();
		}
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
		gotoxy(0, kc_rowForDataRow(i));
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
		cprintf(kc_labelForRow(i));
	}
	initDiceDisplay();
}

void startgame()
{

	char i;

	clrscr();
	textcolor(colSplash);

	cputs("### kkniffel v2.3 ###\r\n\r\n"
		  "written by herr k. @ k-burg, 2019-20\r\n\r\n"
		  "with special thanks to frau k.,\r\n"
		  "buba k., candor k., and - of course -\r\n"
		  "to the 7 turtles!\r\n");

	do
	{
		gotoxy(0, 9);
		cclear(39);
		gotoxy(0, 9);
		textcolor(colText);
		cprintf("# of players (2-4)? ");
		input(inbuf);
		numPlayers = atoi(inbuf);
		if (numPlayers == 0)
		{
			numPlayers = 4;
			cprintf("-> 4");
		}
	} while (numPlayers < 2 || numPlayers > 4);

	namelength = (21 / numPlayers) - 1;
	cputsxy(0,20,"(add 'shift+z' to player name to\r\ncreate a computer player!)");

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
					cprintf("-> katja Z");
					strcpy(inbuf, "katja Z");
				}
				if (i == 1)
				{
					cprintf("-> stephan Z");
					strcpy(inbuf, "stephan Z");
				}
				if (i == 2)
				{
					cprintf("-> buba Z");
					strcpy(inbuf, "buba Z");
				}
				if (i == 3)
				{
					cprintf("-> schnitzel Z");
					strcpy(inbuf, "schnitzel Z");
				}
			}
		} while (strlen(inbuf) == 0);
		if (strchr(inbuf, 'Z'))
		{
			cputs(" (cp) ");
			kc_setIsComputerPlayer(i, true);
		}
		strcpy(_pname[i], inbuf);
	}
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
	jn = cgetc();
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
	updatePlayer(0);
	eraseDice();
	gotoxy(0, 0);
	cputs("          ");
	if (!kc_getIsComputerPlayer(_currentPlayer))
	{
		centerLower("<return> = start rolling");
		waitkey('\n');
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
	unsigned int t;

	if (!kc_getIsComputerPlayer(_currentPlayer))
	{
		if (!shouldCommitRow(row))
		{
			return;
		}
	}

	rOn = 0;

	kc_commitRow(row);
	removeTvalDisplay();
	updateSumDisplay();

	for (i = 0; i < 6; ++i)
	{
		t = getJiffies();
		rOn = !rOn;
		revers(rOn);
		displayTableEntry(_currentPlayer, row, kc_tableValue(row, _currentPlayer, 0), 0);
		while ((getJiffies() - t) < 5)
			;
	}
	revers(0);
	checkQuit();

	if (!quit)
	{
		doNextPlayer();
	}
}

void postRound()
{
	unsigned char i;
	unsigned char j;
	char jn;
	clrscr();
	cprintf("*** round finished! ***");
	updatePlayer(3);
	for (j = 0; j < numPlayers; j++)
	{
		roundResults[currentRound][j] = kc_tableValue(17, j, 0);
		totals[j] = 0;
	}
	for (i = 0; i <= currentRound; i++)
	{
		gotoxy(0, 5 + i);
		cprintf("round %d", i + 1);
		for (j = 0; j < numPlayers; j++)
		{
			gotoxy(columnForPlayer(j), 5 + i);
			cprintf("%d", roundResults[i][j]);
			totals[j] = totals[j] + roundResults[i][j];
		}
	}
	gotoxy(0, 7 + currentRound);
	cputs("total");
	for (j = 0; j < numPlayers; j++)
	{
		gotoxy(columnForPlayer(j), 5 + currentRound + 2);
		cprintf("%d", totals[j]);
	}

	gotoxy(0, 5 + currentRound + 5);
	cputs("another round? (y/n)");
	clearbuf();
	jn = cgetc();
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

void jiffySleep(int num)
{
	unsigned int t;
	t = getJiffies();
	while ((getJiffies() - t) < num)
		;
}

void doCP()
{
	int exitVal = 0;

	centerLower("thinking...");
	cp_analyze();
	jiffySleep(80);
	gotoxy(0, 0);
	clearLower();
	if (kc_getRollCount() == 3)
	{
		exitVal = cp_exitRow();
		commitRow(exitVal);
	}
	else
	{
		exitVal = cp_markDice();
		showCurrentRoll();
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
			gotoxy(0, 0);
			textcolor(colCurrentRollIdx);
			cprintf("(roll %d/%d)", kc_getRollCount(), MAX_ROLL_COUNT);
			plotDiceLegend(kc_getRollCount() < MAX_ROLL_COUNT);
			clearbuf();

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

				cmd = cgetc();
#ifdef DEBUG
				if (cmd == 'A')
				{
					doCP();
				}
				if (cmd == 'D')
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
				if (cmd == '\n' && kc_canRoll())
				{
					doTurnRoll();
				}
				if (cmd == 'q')
					quit = true;
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

void splash()
{
	startup();
	gotoxy(12 + 0, 11);
	cprintf("stephan   katja");
	textcolor(2);
	gotoxy(12 + 8, 11);
	cputc(211);
	jiffySleep(23);
	initIO();
	clrscr();
}

int main()
{
	splash();
	srand(getJiffies());
	mainloop();
	return 0;
}
