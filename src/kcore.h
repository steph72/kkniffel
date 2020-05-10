#ifndef __kcore
#define __kcore

#define false 0
#define true !false

typedef unsigned char byte;

typedef enum _kRow
{
	row_ones,
	row_twos,
	row_threes,
	row_fours,
	row_fives,
	row_sixes,
	row_upper_sum,
	row_upper_bonus,
	row_upper_total,
	row_3same,
	row_4same,
	row_sm_straight,
	row_lg_straight,
	row_full_house,
	row_kniffel,
	row_chance,
	row_lower_sum,
	row_overall_sum
} kRow;

extern int tvals[18];
extern int scores[4];
extern char _currentPlayer;
extern char _currentRound;
extern char _numPlayers;
extern char _numRounds;
extern char _pname[4][20];
extern char _quit;

extern char numbuf[6];

/* initialize a new game */
void kc_initGame(int numPlayers, int numRounds);

void kc_recalcOverallScores(void);
int kc_incrementAndGetSessionCount(void);
int dcompare(const void *a, const void *b);

char kc_newRound(void);

void kc_newTurn(void);
void kc_newRoll(void);

char kc_saveCurrentState(char *fname);
char kc_loadCurrentState(char *fname);
char kc_hasSavedState(char *fname);
void kc_removeCurrentState(char *fname);

char kc_checkQuit(void); /* check if all fields set */
char kc_canRoll(void);   /*       if can roll       */

byte kc_getRollCount(void);

char *kc_currentPlayerName(void);
char *kc_labelForRow(char rowIdx);
byte kc_diceValue(char diceIndex);
int kc_tableValue(char row, char player, char round);
int kc_tempValue(char row);

int kc_getWinner(void);
char *kc_sortedPlayers(void);

char kc_getShouldRoll(unsigned char nr);
void kc_toggleShouldRoll(unsigned char nr);
void kc_setShouldRoll(unsigned char nr, unsigned char val);
void kc_removeShouldRoll(void);
void kc_doSingleRoll(void);
char kc_hasChosenRerollDice(void);

char kc_getIsComputerPlayer(int no);
void kc_setIsComputerPlayer(int no, int isCP);

void kc_commitSort(void);
void kc_recalcTVals(void); /* recalc temp values */

void kc_commitRow(unsigned char row);

char kc_rowForDataRow(unsigned char dataRow);

char kc_getNumTurnsForPlayer(unsigned char playerIdx);

unsigned char checkSame(char count);
byte currentDiceSum(void);

#ifdef DEBUG
void kc_debugFill(char num);
void kc_setDiceValue(char diceIndex, char diceValue);
#endif

#endif