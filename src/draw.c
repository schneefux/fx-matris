#include "sprites.h"
#include "global.h"

//also from ML, a bit modified

void pixel(char *buffer, int x, int y, int type)
{
	if(x&~127 || y&~63) return;
	switch (type)
	{
		case 0:
			(*(buffer + (y << 4) + (x >> 3))) &= ( ~ (128 >> (x & 7)));
			(*(1024 + buffer + (y << 4) + (x >> 3))) &= ( ~ (128 >> (x & 7)));
			(*(2048 + buffer + (y << 4) + (x >> 3))) &= ( ~ (128 >> (x & 7)));
			break;
		case 1:
			(*(2048 + buffer +  (y << 4) + (x >> 3))) |= (128 >> (x & 7));
			(*(1024 + buffer +  (y << 4) + (x >> 3))) |= (128 >> (x & 7));
			(*(buffer +  (y << 4) + (x >> 3))) |= (128 >> (x & 7));
			break;
	}
}

void pixelgray(char *buffer, int x,int y, int type)
{
	switch (type)
	{
		case 0:
			(*(buffer + (y << 4) + (x >> 3))) &= ( ~ (128 >> (x & 7)));
			(*(1024 + buffer + (y << 4) + (x >> 3))) &= ( ~ (128 >> (x & 7)));
			(*(2048 + buffer + (y << 4) + (x >> 3))) &= ( ~ (128 >> (x & 7)));
			break;
		case 3:
			(*(2048 + buffer +  (y << 4) + (x >> 3))) |= (128 >> (x & 7));
		case 2:
			(*(1024 + buffer +  (y << 4) + (x >> 3))) |= (128 >> (x & 7));
		case 1:
			(*(buffer +  (y << 4) + (x >> 3))) |= (128 >> (x & 7));
			break;
	}
}

#define Abs(x) ((x < 0)? -x : x)
#define sgn(x) (x == 0)?0:(x > 0)?1:-1

void line(char *buffer, int x1, int y1, int x2, int y2, int color)
{
	int i, x, y, dx, dy, sx, sy, cumul;
	
	y1 = y1 + 7;
	y2 = y2 + 7;
	
	x = x1;
	y = y1;
	dx = x2 - x1;
	dy = y2 - y1;
	sx = sgn(dx);
	sy = sgn(dy);
	dx = Abs(dx);
	dy = Abs(dy);
	pixel(buffer, x, y, color);
	if(dx > dy)
	{
		cumul = dx / 2;
		for(i=1 ; i<dx ; i++)
		{
			x += sx;
			cumul += dy;
			if(cumul > dx)
			{
				cumul -= dx;
				y += sy;
			}
			pixel(buffer, x, y, color);
		}
	}
	else
	{
		cumul = dy / 2;
		for(i=1 ; i<dy ; i++)
		{
			y += sy;
			cumul += dx;
			if(cumul > dy)
			{
				cumul -= dy;
				x += sx;
			}
			pixel(buffer, x, y, color);
		}
	}
}
//end of ML stuff

void drawChar(char *buffer, char charact, int posx, int posy)
{
	int xcount, ycount;
	
	for(xcount = 0; xcount < 5; xcount ++)
	{
		for(ycount = 0; ycount < 3; ycount ++)
		{
			pixel(buffer, posx + xcount, posy - ycount, character[charact - 65][xcount][ycount]);
			//this should be optimized: it is drawed pixel per pixel
		}
	}
}

void drawString(char *buffer, char *string, unsigned char strlenght, int posx, int posy)
{
	int ycount, charcount;
	
	for(charcount = strlenght - 1 , ycount = posy; charcount >= 0; charcount --, ycount += 4)
	{
		drawChar(buffer, *(string + charcount), posx, ycount);
		//every char seperately
	}
}

void drawNumber(char *buffer, char num, int posx, int posy)
{
	int xcount, ycount;
	
	for(xcount = 0; xcount < 5; xcount ++)
	{
		for(ycount = 0; ycount < 3; ycount ++)
		{
			pixel(buffer, (int)posx + xcount, (int)posy - ycount, (int)numbers[num][xcount][ycount]);
			//like drawChar
		}
	}
}

void fillbox(char *buffer, int posx, int posy, int type)
{
	int ycount, xcount;
	
	for(ycount = 0; ycount <= 4; ycount ++)
	{
		for(xcount = 0; xcount <= 4; xcount ++)
		{
			pixelgray(buffer, posx * 5 + xcount + 1, posy * 5 + ycount + 1 + 7, spritesheed[type][ycount][xcount]);
			//draw a brick box pixel per pixel
		}
	}
}

void normalfillbox(char *buffer, int posx, int posy, int type)
{
	int ycount, xcount;
	
	for(ycount = 0; ycount <= 4; ycount ++)
	{
		for(xcount = 0; xcount <= 4; xcount ++)
		{
			pixelgray(buffer, posx * 5 + xcount - 2, posy * 5 + ycount, spritesheed[type][ycount][xcount]);
			//like fillbox, just a different position
		}
	}
}

int DrawSprite()
{
	char fit = True;
	
	int xcount, ycount;
	right = True;
	left = True;	 
	
	memset(&changemap, 0, 210);

	//now draw a brick, the sizes are different
	//structs and a drawing function would be better, it would save code

	switch(block)
	{
		case 1: 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockO[dir][ycount][xcount] == 1 && ypos - ycount - 2 >= 0)
					{
						if(xpos - xcount - 2 < 0 || xpos - xcount - 2 > 9 || ypos - ycount - 2 > 20)
						{
							fit = False;
						}
						else
						{
							changemap[ypos - ycount - 2][xpos - xcount - 2] = 1;
							if(globalmap[ypos - ycount - 2][xpos - xcount - 2] != 0 || ypos - ycount - 2 >= 20)
							{
								fit = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount - 1 - 2] != 0 || xpos - xcount - 2 < 1)
							{
								right = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount + 1 - 2] != 0 || xpos - xcount - 2 > 8)
							{
								left = False;
							}
						}
					}
				}
			}
			break;
		case 2: 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockI[dir][ycount][xcount] == 1 && ypos - ycount - 2 >= 0)
					{
						if(xpos - xcount - 2 < 0 || xpos - xcount - 2 > 9 || ypos - ycount - 2 > 20)
						{
							fit = False;
						}
						else
						{
							changemap[ypos - ycount - 2][xpos - xcount - 2] = 2;
							if(globalmap[ypos - ycount - 2][xpos - xcount - 2] != 0 || ypos - ycount - 2 >= 20)
							{
								fit = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount - 1 - 2] != 0 || xpos - xcount - 2 < 1)
							{
								right = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount + 1 - 2] != 0 || xpos - xcount - 2 > 8)
							{
								left = False;
							}
						}
					}
				}
			}
			break;
		case 3: 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockT[dir][ycount][xcount] == 1&& ypos - ycount - 2 >= 0)
					{
						if(xpos - xcount - 2 < 0 || xpos - xcount - 2 > 9 || ypos - ycount - 2 > 20)
						{
							fit = False;
						}
						else
						{
							changemap[ypos - ycount - 2][xpos - xcount - 2] = 3;
							if(globalmap[ypos - ycount - 2][xpos - xcount - 2] != 0 || ypos - ycount - 2 >= 20)
							{
								fit = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount - 1 - 2] != 0 || xpos - xcount - 2 < 1)
							{
								right = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount + 1 - 2] != 0 || xpos - xcount - 2 > 8)
							{
								left = False;
							}
						}
					}
				}
			}
			break;
		case 4: 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockJ[dir][ycount][xcount] == 1 && ypos - ycount - 2 >= 0)
					{
						if(xpos - xcount - 2 < 0 || xpos - xcount - 2 > 9 || ypos - ycount - 2 > 20)
						{
							fit = False;
						}
						else
						{
							changemap[ypos - ycount - 2][xpos - xcount - 2] = 4;
							if(globalmap[ypos - ycount - 2][xpos - xcount - 2] != 0 || ypos - ycount - 2 >= 20)
							{
								fit = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount - 1 - 2] != 0 || xpos - xcount - 2 < 1)
							{
								right = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount + 1 - 2] != 0 || xpos - xcount - 2 > 8)
							{
								left = False;
							}
						}
					}
				}
			}
			break;
		case 5: 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockL[dir][ycount][xcount] == 1 && ypos - ycount - 2 >= 0)
					{
						if(xpos - xcount - 2 < 0 || xpos - xcount - 2 > 9 || ypos - ycount - 2 > 20)
						{
							fit = False;
						}
						else
						{
							changemap[ypos - ycount - 2][xpos - xcount - 2] = 5;
							if(globalmap[ypos - ycount - 2][xpos - xcount - 2] != 0 || ypos - ycount - 2 >= 20)
							{
								fit = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount - 1 - 2] != 0 || xpos - xcount - 2 < 1)
							{
								right = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount + 1 - 2] != 0 || xpos - xcount - 2 > 8)
							{
								left = False;
							}
						}
					}
				}
			}
			break;
		case 6: 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockZ[dir][ycount][xcount] == 1 && ypos - ycount - 2 >= 0)
					{
						if(xpos - xcount - 2 < 0 || xpos - xcount - 2 > 9 || ypos - ycount - 2 > 20)
						{
							fit = False;
						}
						else
						{
							changemap[ypos - ycount - 2][xpos - xcount - 2] = 6;
							if(globalmap[ypos - ycount - 2][xpos - xcount - 2] != 0 || ypos - ycount - 2 >= 20)
							{
								fit = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount - 1 - 2] != 0 || xpos - xcount - 2 < 1)
							{
								right = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount + 1 - 2] != 0 || xpos - xcount - 2 > 8)
							{
								left = False;
							}
						}
					}
				}
			}
			break;
		case 7: 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockS[dir][ycount][xcount] == 1 && ypos - ycount - 2 >= 0)
					{
						if(xpos - xcount - 2 < 0 || xpos - xcount - 2 > 9 || ypos - ycount - 2 > 20)
						{
							fit = False;
						}
						else
						{
							changemap[ypos - ycount - 2][xpos - xcount - 2] = 7;
							if(globalmap[ypos - ycount - 2][xpos - xcount - 2] != 0 || ypos - ycount - 2 >= 20)
							{
								fit = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount - 1 - 2] != 0 || xpos - xcount - 2 < 1)
							{
								right = False;
							}
							if(globalmap[ypos - ycount - 2][xpos - xcount + 1 - 2] != 0 || xpos - xcount - 2 > 8)
							{
								left = False;
							}
						}
					}
				}
			}
			break;
	}
	return fit;
}

void DrawBlock()
{
	int xcount, ycount;
		
	clrbuffer(pScreen);	
	
	drawNumber(pScreen, (int)(level / 10) % 10, 115, 39);
	drawNumber(pScreen, (int)(level / 10) % 100 / 10, 115, 43);
	
	drawString(pScreen, &"LV", 2, 115, 59);
	
	drawNumber(pScreen, points % 10, 109, 39);
	drawNumber(pScreen, points % 100 / 10, 109, 43);
	drawNumber(pScreen, points % 1000 / 100, 109, 47);
	drawNumber(pScreen, points % 10000 / 1000, 109, 51);
	drawNumber(pScreen, points % 100000 / 10000, 109, 55);
	drawNumber(pScreen, points % 1000000 / 100000, 109, 59);
	drawNumber(pScreen, points / 1000000, 109, 63);
	
	drawString(pScreen, &"PTS", 3, 103, 51);
	
	drawString(pScreen, &"LINE", 4, 121, 51);
	
	drawNumber(pScreen, level % 10, 121, 39);
	drawNumber(pScreen, level % 100 / 10, 121, 43);
	drawNumber(pScreen, level % 1000 / 100, 121, 47);
		
	if(DrawSprite() == False)
	{		
		for(ycount = 0; ycount < 20; ycount ++)
		{
			for(xcount = 0; xcount < 10; xcount ++)
			{
				if(changemap[ycount + 1][xcount] != 0)
				{
					globalmap[ycount][xcount] = changemap[ycount + 1][xcount];
				}
			}
		}
		CreateBlock();
		if(nextdir == Fast)
		{
			nextdir = Down;
		}
	}

	line(pScreen, 0 , 0 , 101 , 0, 1);
	line(pScreen, 0 , 0 , 0 , 51, 1);
	line(pScreen, 101 , 0 , 101 , 51, 1);
	line(pScreen, 101 , 51 , 0 , 51, 1);

	for(ycount = 0; ycount < 20; ycount ++)
	{
		for(xcount = 0; xcount < 10; xcount ++)
		{
			if(globalmap[ycount][xcount] != 0 || changemap[ycount][xcount] != 0)
			{
				fillbox(pScreen, ycount, xcount,  (globalmap[ycount][xcount] != 0)? globalmap[ycount][xcount] - 1 : changemap[ycount][xcount] - 1);
			}
		}
	}

	drawString(pScreen, &"NEXT", 4, 103, 2);
	
	//draw the next block
	
	switch(nextblock)
	{
		case 1 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockO[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 0 + xcount, 0);
					}
				}
			}
			break;
			
		case 2 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockI[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 0 + xcount, 1);
					}
				}
			}
			break;
			
		case 3 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockT[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 0 + xcount, 2);
					}
				}
			}
			break;
			
		case 4 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockJ[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 0 + xcount, 3);
					}
				}
			}
			break;

		case 5 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockL[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 0 + xcount, 4);
					}
				}
			}
			break;
			
		case 6 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockZ[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 0 + xcount, 5);
					}
				}
			}
			break;
			
		case 7 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockS[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 0 + xcount, 6);
					}
				}
			}
			break;
	}
	
	drawString(pScreen, &"HOLD", 4, 103, 22);
	
	//draw the stored block
	
	switch(hold)
	{
		case 1 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockO[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 4 + xcount, 0);
					}
				}
			}
			break;
			
		case 2 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockI[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 4 + xcount, 1);
					}
				}
			}
			break;
			
		case 3 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockT[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 4 + xcount, 2);
					}
				}
			}
			break;
			
		case 4 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockJ[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 4 + xcount, 3);
					}
				}
			}
			break;

		case 5 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockL[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 4 + xcount, 4);
					}
				}
			}
			break;
			
		case 6 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockZ[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 4 + xcount, 5);
					}
				}
			}
			break;
			
		case 7 : 
			for(xcount = 0; xcount < 3; xcount ++)
			{
				for(ycount = 0; ycount < 3; ycount ++)
				{
					if(blockS[0][ycount][xcount] == 1)
					{
						normalfillbox(pScreen, 22 + ycount , 4 + xcount, 6);
					}
				}
			}
			break;
	}	
}
