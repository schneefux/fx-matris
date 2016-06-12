#ifndef __DRAW_H__
#define __DRAW_H__

void pixel(char *buffer, int x, int y, int type);
void pixelgray(char *buffer, int x,int y, int type);
void line(char *buffer, int x1, int y1, int x2, int y2, int color);
void drawChar(char *buffer, char charact, int posx, int posy);
void drawString(char *buffer, char *string, unsigned char strlenght, int posx, int posy);
void drawNumber(char *buffer, char num, int posx, int posy);
void fillbox(char *buffer, int posx, int posy, int type);
void normalfillbox(char *buffer, int posx, int posy, int type);
int DrawSprite();
void DrawBlock();

#endif