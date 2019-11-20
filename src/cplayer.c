
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

*/	
	


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kcore.h"

int  cp_scoreForRowChoice[18];
char cp_sortedRerollRows[18];
char cp_haveBonus;

int currentValueForRow(int aRow) {
	return kc_tableValue(aRow,_currentPlayer,_currentRound);
}


void analyzeUpperRows(void) {
	char row;
	int currentValue;
	int possibleValue;
	int scoreV;
		
	cp_haveBonus = (currentValueForRow(row_upper_bonus)>0);
		
	for (row=0;row<6;row++) {
		currentValue = currentValueForRow(row);
		if (currentValue==-1) {
		
			possibleValue = tvals[row];
			
			/* 
					max scoring for upper rows:
			   		30 (6*5) + 50 + 30 = 120			
			*/
			
			scoreV = (possibleValue/(row+1))*10;
			
			
			/* cannot roll & row not sufficient? rate low! */
			
			if (kc_getRollCount()==3) {
			
				if (scoreV<30) {
					cp_scoreForRowChoice[row]=5;
				} else {
					cp_scoreForRowChoice[row]=scoreV+possibleValue;
				}
				
			} else {
			
				if (scoreV>=30) {				/* rate 'sufficient' rows higher */
					scoreV += possibleValue;
					if (!cp_haveBonus) {		/* ...and even higher if */
						scoreV += 30;			/* there is no bonus yet */ 
					}
				}
				cp_scoreForRowChoice[row] = scoreV;
			}
		}
	}
}

void analyzeSameKind(void) {
	char row;
	char dCountCheck;
	int currentValue;
	int sameDiceValue;
	int scoreV;
	for (row=9;row<11;row++) {
		currentValue = currentValueForRow(row);
		if (currentValue==-1) {
			scoreV = 0;
			for (dCountCheck=2;dCountCheck<=5;dCountCheck++) {
				sameDiceValue = checkSame(dCountCheck);
				if (sameDiceValue) {
					scoreV = sameDiceValue*dCountCheck; /* *2 */
					if (row==9 && dCountCheck>=3) {
						scoreV+=10;
						break;
					}	
					if (row==10 && dCountCheck>=4) {
						scoreV+=20;
						break;
					}
				}
			}
			cp_scoreForRowChoice[row] = scoreV;
		}
	}
}

int getConsecutiveDiceCount(char **start) {

	byte i;
	int consec[2];
	static char startElem[2];
	int currentPart;
	int diff;
	unsigned char sorted[5];

	startElem[0]=0;
	startElem[1]=0;
		
	for (i=0;i<5;i++) {
		sorted[i]=kc_diceValue(i);
	}
	qsort(sorted,5,1,dcompare);
	
	consec[0]=0; 
	consec[1]=0; 
	currentPart=0;
	
	for (i=0;i<4;i++) {
		diff = sorted[i+1]-sorted[i];
		if (diff==1) {
			if (startElem[currentPart]==0) {
				startElem[currentPart]=sorted[i];
			}
			consec[currentPart]++;
		} else if (diff>=2 && consec[currentPart]!=0) {
			currentPart++;
		}
	}
	if (consec[1]>consec[0]) {
		*start = &startElem[1];
		return consec[1];
	}
	
	*start = &startElem[0]; 
	return consec[0];
}


void analyzeStraight(int row) {
	int currentValue;
	int tempScore = 0;
	int consec;
	char *startElemPtr;

	currentValue = currentValueForRow(row);
	
	if (currentValue==-1) {
		consec = getConsecutiveDiceCount(&startElemPtr);
		/* gotoxy(81,5);
		printf("c/SE: %d/%d ",consec,*startElemPtr); */
		if (row==11 && consec>=3) {
			tempScore=50;
		} else if (row==12 && consec==3 && kc_getRollCount()<3) {
			tempScore=55;
		} else if (row==12 && consec==4) {
			tempScore=52;
		} else {
			tempScore=consec*10;
			if (kc_getRollCount()==2 && tempScore>=10) {
				tempScore-=10;
			} else if (kc_getRollCount()==3) {
				tempScore=0;
			}
		}
		cp_scoreForRowChoice[row] = tempScore;
	}
}

void analyzeFullHouse(char row) {
	int currentValue;
	int tempScore =0;
	int twoSame, threeSame;
	
	currentValue = currentValueForRow(row);
	if (currentValue==-1) {
		threeSame = checkSame(3);
		twoSame = checkSame(2);
		if (twoSame && threeSame && (threeSame!=twoSame)) {
			tempScore=60;
			if (currentDiceSum()<=15) {
				tempScore+=20;
			}
		} else if (threeSame && !twoSame) {
			tempScore=40;
		}
		cp_scoreForRowChoice[row] = tempScore;
	}
}

void analyzeKniffel(int row) {
	int currentValue;
	int same;
	int i;
	int score;
	currentValue = currentValueForRow(row);
	if (currentValue>=0) {
		return;
	}
	for (i=5;i>2;i--) {
		same = checkSame(i);
		if (same) {
			if (i==5) {
				cp_scoreForRowChoice[row]=200;
				return;
			}
			score = 15*i;
			score += (3-kc_getRollCount())*10;
			cp_scoreForRowChoice[row]=score;
			return;
		}
	}
}

int scoreSort(const void *_a, const void *_b) {
	int score1;
	int score2;
	char *a,*b;
	a = (char*)_a;
	b = (char*)_b;
	score1 = cp_scoreForRowChoice[*a];
	score2 = cp_scoreForRowChoice[*b];
	if (score1<score2) return 1;
	if (score1>score2) return -1;
	if (*a>*b) return 1;
	if (*a<*b) return -1;
	return 0;
} 

int markDiceForUpperSection(int aRow) {
	byte i;
	int dValue = aRow+1;
	int currentValue;
	currentValue = tvals[aRow];
	if (currentValue==((aRow+1)*5)) {
		return aRow;
	}
	if (currentValue>=((aRow+1)*3) && kc_getRollCount()==3) {
		return aRow;
	}	
	if (kc_getRollCount()==3) {
		return -1;
	}
	for (i=0;i<5;i++) {
		kc_setShouldRoll(i,kc_diceValue(i)!=dValue);
	}	
	return -1;
}

int markDiceForSame(int row) {
	int destVal = 0;
	int testVal = 0;
	int destI =0;
	byte i;
	int currentValue;
	currentValue = tvals[row];
	if (currentValue>0) {
		return row;
	}	
	for (i=2;i<5;i++) {
		testVal = checkSame(i);
		if ((testVal*i)>(destVal*destI)) {
			destVal=testVal;
			destI=i;
		}	
	}
	for (i=0;i<5;++i) {
		kc_setShouldRoll(i,kc_diceValue(i)!=destVal);
	}
	return -1;
}

int markDiceForFullHouse(void) {
	int valToKeep;
	byte i;
	int currentValue;
	currentValue = tvals[row_full_house];
	if (currentValue>0) {
		return row_full_house;
	}
	valToKeep = checkSame(3);
	if (!valToKeep) {
		valToKeep=checkSame(2);
	}
	
	/* mark all dice that are not in found 2 or 3) */
	for (i=0;i<5;++i) {
		kc_setShouldRoll(i,kc_diceValue(i)!=valToKeep);
	}
	
	/* we need one less to roll in any case */
	
	for (i=0;i<5;i++) {
		if (kc_getShouldRoll(i)) {
			kc_toggleShouldRoll(i);
			break;
		}
	}
	return -1;
}


int markDiceForStraight(int row) {
	char *startElem;
	char diceValueCount[6];
	char consecCount;
	char currentDiceVal;
	char i;
	char shouldRoll;
	int currentValue;
	currentValue = tvals[row];
	if (row>=row_4same && currentValue>0) {
		return row;
	}
	if (row==row_3same && kc_getRollCount()==3) {
		return row;
	}
	consecCount = getConsecutiveDiceCount(&startElem);
	memset (diceValueCount,0,6);
	for (i=0;i<5;i++) {
		currentDiceVal = kc_diceValue(i);
		diceValueCount[currentDiceVal]++;
		shouldRoll = currentDiceVal<*startElem || currentDiceVal>(*startElem)+consecCount;
		shouldRoll |= diceValueCount[currentDiceVal]>1;	
		kc_setShouldRoll(i,shouldRoll);
	}
	return -1;
}

int cp_markDice(void) {
	int row;
	row = cp_sortedRerollRows[0];
	kc_recalcTVals();
	if (row<=5) {
		return markDiceForUpperSection(row);
	} else if (row==row_kniffel || row==row_3same || row==row_4same) {
		return markDiceForSame(row);
	} else if (row==row_sm_straight || row==row_lg_straight) {
		return markDiceForStraight(row);
	} else if (row==row_full_house) {
		return markDiceForFullHouse();
	}
	return -1;
}


void cp_analyze(void) {
	int i;	
	for (i=0;i<18;i++) {
		cp_sortedRerollRows[i]=i;
		cp_scoreForRowChoice[i]=-1;
	}
	kc_recalcTVals();
	analyzeUpperRows();
	analyzeSameKind();
	analyzeStraight(11);
	analyzeStraight(12);
	analyzeFullHouse(13);
	analyzeKniffel(14);
	qsort(cp_sortedRerollRows,18,1,scoreSort);
	/* debugDumpCP(); */
}

int cp_exitRow(void) {

	int i;
	int currentValue;
	int haveUpper;
	int upperSum=0;
	int pointsNeededForBonus=0;
	int lowerMod;
	
	/* determine if we have all upper rolls... */
	haveUpper=true;
	for (i=0;i<=5;i++) {
		currentValue = currentValueForRow(i);
		if (currentValue<0) {
			haveUpper=false;
		} else {
			upperSum+=currentValue;
		}
	}
	pointsNeededForBonus = 63-upperSum;
	for (i=0;i<18;i++) {
		cp_sortedRerollRows[i]=i;
		currentValue = currentValueForRow(i);
		
		/* slot is still free? */
		if (currentValue<0 && i!=row_upper_bonus) {
		
			/* is it an upper slot? */
			if (i<=5) {
		
				/* enough for bonus? then prefer this slot */
				if (tvals[i]>=pointsNeededForBonus) {
					cp_scoreForRowChoice[i]+=50;
				} else {
		
					/* make slot available for worst case */
					if (tvals[i]<((i+1)*2)) {
						cp_scoreForRowChoice[i]=5;
					}
				}
		
			} else {
			 	/* lower slot */
			 	if (tvals[i]==0 && i>=row_3same) {
			 		lowerMod = i-row_3same;
			 		if (i==row_full_house) {
			 			lowerMod = 0;
			 		}
			 		if (haveUpper || pointsNeededForBonus<=0) {
			 			cp_scoreForRowChoice[i]=5+lowerMod;
			 		} else {
			 			/* give lower slots away more easily
			 				while still aiming for bonus */
			 			cp_scoreForRowChoice[i]=6+lowerMod;
			 		}
			 		if (i==row_kniffel) {
			 			cp_scoreForRowChoice[i]++;
			 		}
			 	}
			}
		}
	}
	
	if (cp_scoreForRowChoice[0]==5) {
		cp_scoreForRowChoice[0]=8 + tvals[0];
	}
	if (cp_scoreForRowChoice[1]==5) {
		cp_scoreForRowChoice[1]=8 + tvals[1];
	}
	
	if (currentValueForRow(row_chance)<0) {
		cp_scoreForRowChoice[row_chance] = 14;
		if (tvals[row_chance]>19) {
			cp_scoreForRowChoice[row_chance] = 17;
		}
	}
	
	qsort(cp_sortedRerollRows,18,1,scoreSort);
	return cp_sortedRerollRows[0];
}