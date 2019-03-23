
void initIO(void);
void startup(void);

void __fastcall__ _plotDice(unsigned char value, unsigned char x, unsigned char y, char r);
void plotDice(unsigned char nr, unsigned char value, char revers);
void eraseDie(unsigned char nr);

void initDiceDisplay(void);

extern const char dice[6][25];

extern const unsigned char colBackground;
extern const unsigned char colBorder;
extern const unsigned char colText;

extern const unsigned char colSplash;
extern const unsigned char colSplashRed;

extern const unsigned char colTable;
extern const unsigned char colLegend;

extern const unsigned char colTempValue;
extern const unsigned char colEvenValue;
extern const unsigned char colOddValue;
extern const unsigned char colUpperSum;        // upper sum
extern const unsigned char colLowerSum;        // lower sum
extern const unsigned char colBonus;           // bonus
extern const unsigned char colCurrentRollIdx;
