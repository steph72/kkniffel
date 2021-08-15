#include "highscores.h"
#include <stdio.h>
#include <string.h>
#include "congui.h"

static char hsNames[20][HS_LISTSIZE];
static int hsScores[HS_LISTSIZE];

void setDefaultHighscores(void)
{
    unsigned char i;
    for (i = 0; i < HS_LISTSIZE; ++i)
    {
        strcpy(hsNames[i],"Candulf");
        hsScores[i] = 150-(i*10);
    }
}

char *highscoreAtPos(char pos)
{
    static char line[40];
    sprintf(line, "%2d. %s (%d)", pos, hsNames[pos - 1], hsScores[pos - 1]);
    return line;
}

void insertNewScore(int score, int pos, char *name)
{
    unsigned char i;
    if (i < HS_LISTSIZE-1)
    {
        for (i = HS_LISTSIZE - 1; i != pos; --i)
        {
            hsScores[i] = hsScores[i - 1];
            strcpy(hsNames[i], hsNames[i - 1]);
        }
    }
    strcpy(hsNames[pos], name);
    hsScores[pos] = score;
}

// returns hs position or 0 if no new highscore
int checkAndCommitHighscore(int score, char *name)
{
    unsigned char i;
    for (i = 0; i < HS_LISTSIZE; ++i)
    {
        if (score > hsScores[i])
        {
            insertNewScore(score, i, name);
            return i + 1;
        }
    }
    return 0;
}

void loadHighscores(FILE *scorefile)
{
    fread(hsNames, sizeof(hsNames), 1, scorefile);
    fread(hsScores, sizeof(hsScores),1,scorefile);
}

void saveHighscores(void)
{
    FILE *scorefile;
    scorefile = fopen("kkscr", "wb");
    fwrite(hsNames, sizeof(hsNames), 1, scorefile);
    fwrite(hsScores, sizeof(hsScores),1,scorefile);
    fclose(scorefile);
}

void initHighscores(void)
{
    FILE *scorefile;
    scorefile = fopen("kkscr", "rb");
    if (!scorefile)
    {
        setDefaultHighscores();
    }
    else
    {
        loadHighscores(scorefile);
    }
    fclose(scorefile);
}