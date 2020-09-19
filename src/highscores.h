#define HS_LISTSIZE 10

int checkAndCommitHighscore(int score, char *name);
char *highscoreAtPos(char pos);
void initHighscores(void);
void saveHighscores(void);