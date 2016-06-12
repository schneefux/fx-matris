#include "global.h"

char* pScreen;
//pointer to the 3 graybuffers

char right = True;
char left = True;
//can move?

char changemap[21][10] = {0};
char globalmap  [20][10] = {0};
//the two brickmaps: globalmap is the visible one

char block;
//block type
int xpos = 7, ypos = 3;
char dir;
//position and direction of brick

char level;
//current level

unsigned int points = 0;
//points

char nextdir = Down;
//the direction for the next round

char nextblock = 1;
//the next block

char hold = 0;
//the stored brick
