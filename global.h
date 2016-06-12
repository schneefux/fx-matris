#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define Dir_0     0
#define Dir_90   1
#define Dir_180 2
#define Dir_270 3

//direction of brick

#define Down 0
#define Left   -1
#define Right  1
#define Fast    30
#define Rot     20
#define Swap  10

//type of movement

#define False 0
#define True  1

//for bools

extern char* pScreen;
//pointer to the 3 graybuffers

extern char right;
extern char left;
//can move?

extern char changemap[21][10];
extern char globalmap  [20][10];
//the two brickmaps: globalmap is the visible one

extern char block;
//block type
extern int xpos, ypos;
extern char dir;
//position and direction of brick

extern int level;
//current level

extern unsigned int points;
//points

extern char nextdir;
//the direction for the next round

extern char nextblock;
//the next block

extern char hold;
//the stored brick

#endif