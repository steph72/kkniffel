void initPalette(void);

void _plotDice(unsigned char value, unsigned char x, unsigned char y, char r);
void plotDice(unsigned char nr, unsigned char value, char revers);
void eraseDie(unsigned char nr);

#define DICE_X_POS 71
#define DICELEGEND_POS DICE_X_POS-2

extern const char *gTitle;
extern const char dice[6][40];

extern const unsigned char colBackground;
extern const unsigned char colBorder;
extern const unsigned char colText;
extern const unsigned char colTitleBG;
extern const unsigned char colTitleText;


extern const unsigned char colDice;

extern const unsigned char colSplash;
extern const unsigned char colSplashRed;

extern const unsigned char colLegend;

extern const unsigned char colTempValue;
extern const unsigned char colEvenValue;
extern const unsigned char colOddValue;
extern const unsigned char colUpperSum;        // upper sum
extern const unsigned char colLowerSum;        // lower sum
extern const unsigned char colBonus;           // bonus
extern const unsigned char colCurrentRollIdx;
