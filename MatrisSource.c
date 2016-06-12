#include "fxlib.h"
#include <Sprites.h>
#include <Key.h>

#define Dir_0     0
#define Dir_90   1
#define Dir_180 2
#define Dir_270 3

#define Down 0
#define Left   -1
#define Right  1
#define Fast    30
#define Rot     20
#define Swap  10

#define Startconnection 111
#define Received 123

#define False 0
#define True  1

int KBD_PRGM_GetKey( void*p );
int PRGM_GetKey()
{
	unsigned char buffer[12];
	KBD_PRGM_GetKey( buffer );
	return ( buffer[1] & 0x0F ) * 10 + ( ( buffer[2]  & 0xF0 )  >> 4 );
}

int Serial_BufferedTransmitOneByte( unsigned char ); 
int Serial_Open( void*sm ); 
int Serial_Close( int mode ); 
int Serial_ReadOneByte( unsigned char* ); 
char *Disp_GetVRAMPtr(void);

void PutDisp_DD(void);
void DrawLineVRAM(int x1, int y1, int x2, int y2);
void AllClr_DD(void);
void AllClr_VRAM(void);
void AllClr_DDVRAM(void);
void Sleep(int millisecond);
int GetKey(unsigned int *keycode);
void locate(int x, int y);
void Print(const unsigned char *str);
int Timer_Install(int TimerID, void (*handler)(void), int delay);
int Timer_Uninstall(int TimerID);
int Timer_Start(int TimerID);

char* pScreen;

int level;

int levelchanged = False;

int block;
int xpos = 5, ypos = 0;
int dir;
int exit = False;
int nextdir = Down;

int points = 0;

int nextblock = 1;

int ready;

int onoff = True;

int canswap;

float slowyposdown;

int changemap[21][10] = {0};
int globalmap  [20][10] = {0};

int right = True;
int left = True;

int hold = 0;

int connected;

unsigned char recieveline = 0;
unsigned char sendline     = 0;

unsigned char recieveadd = 0;

int cantrans = False;

void drawbuffer(char *buffer)
{
/*	DISPGRAPH mydisp;
	GRAPHDATA mygraph;
	mygraph.width = 128;
	mygraph.height = 64;
	mygraph.pBitmap = buffer;
	mydisp.x = 0;
	mydisp.y = 0;
	mydisp.GraphData = mygraph;
	mydisp.WriteModify = IMB_WRITEMODIFY_NORMAL;
	mydisp.WriteKind = IMB_WRITEKIND_OVER;
	Bdisp_WriteGraph_VRAM(&mydisp);*/

	int counter;
	char * vram = Disp_GetVRAMPtr();
	
	for(counter = 0; counter < 1024; counter ++)
	{
		*(vram + counter) = *(buffer + counter);
	}
}

void RestGray(void)
{
	drawbuffer(pScreen);
	PutDisp_DD();
	
	drawbuffer(pScreen + 1024);
	PutDisp_DD();
	
	drawbuffer(pScreen + 2048);
	PutDisp_DD();
}

int Abs(int num)
{
	return(num < 0)? num * -1 : num;
}

int round(float num)
{
	if((int) (num + 0.5) != (int) num)
	{
		return (int) (num + 0.5);
	}
	else
	{
		return (int) num;
	}
}

void clrbuffer(char *buffer)
{
	int xcount;
	
	for(xcount = 0; xcount < 3072; xcount ++)
	{
		*(buffer + xcount) = 0;
	}
}

void pixel(char *buffer, int x, int y, int type)
{
	switch (type)
	{
		case 0:
			(*(buffer + y * 16 + (int) (x / 8))) &= ( ~ (128 >> (x % 8)));
			(*(1024 + buffer + y * 16 + (int) (x / 8))) &= ( ~ (128 >> (x % 8)));
			(*(2048 + buffer + y * 16 + (int) (x / 8))) &= ( ~ (128 >> (x % 8)));
			break;
		case 1:
			(*(2048 + buffer + y * 16 + (int) (x / 8))) |= (128 >> (x % 8));
			(*(1024 + buffer + y * 16 + (int) (x / 8))) |= (128 >> (x % 8));
			(*(buffer + y * 16 + (int) (x / 8))) |= (128 >> (x % 8));
			break;
	}
}

void pixelgray(char *buffer, int x, int y, int type)
{
	switch (type)
	{
		case 0:
			(*(buffer + y * 16 + (int) (x / 8))) &= ( ~ (128 >> (x % 8)));
			(*(1024 + buffer + y * 16 + (int) (x / 8))) &= ( ~ (128 >> (x % 8)));
			(*(2048 + buffer + y * 16 + (int) (x / 8))) &= ( ~ (128 >> (x % 8)));
			break;
		case 3:
			(*(2048 + buffer + y * 16 + (int) (x / 8))) |= (128 >> (x % 8));
		case 2:
			(*(1024 + buffer + y * 16 + (int) (x / 8))) |= ( 128 >> (x % 8));
		case 1:
			(*(buffer + y * 16 + (int) (x / 8))) |= (128 >> ((x % 8)));
			break;
	}
}

void line(char *buffer, int x1, int y1, int x2, int y2, int type) 
{ 
	int dx = x2 - x1 , dy = y2 - y1 , steps ,k; 
	
	float xincr,yincr, x=x1 , y=y1; 
	
	if(Abs(dx)>Abs(dy)) steps=Abs(dx); 
	else steps=Abs(dy); 
	
	xincr=dx/(float)steps; 
	yincr=dy/(float)steps; 
	
	pixel(buffer, round(x), round(y), type);
	
	for(k=0;k<steps;k++) { 
		x+=xincr; 
		y+=yincr; 
		pixel(buffer, round(x), round(y), type); 
	} 
} 

void drawChar(char *buffer, char charact, int posx, int posy)
{
	int xcount, ycount;
	
	for(xcount = 0; xcount < 5; xcount ++)
	{
		for(ycount = 0; ycount < 3; ycount ++)
		{
				pixel(buffer, posx + xcount, posy - ycount, character[charact - 65][xcount][ycount]); 
		}
	}
}

void drawString(char *buffer, char * string, int strlenght, int posx, int posy)
{
	int  ycount, charcount;
	
	for(charcount = strlenght - 1 , ycount = posy; charcount >= 0; charcount --, ycount += 4)
	{
		drawChar(buffer, *(string + charcount), posx, ycount);
	}
}

void drawNumber(char *buffer, int num, int posx, int posy)
{
	int xcount, ycount;
	
	for(xcount = 0; xcount < 5; xcount ++)
	{
		for(ycount = 0; ycount < 3; ycount ++)
		{
				pixel(buffer, posx + xcount, posy - ycount, numbers[num][xcount][ycount]); 
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
			pixelgray(buffer, posx * 5 + xcount + 1, posy * 5 + ycount + 1, spritesheed[type][ycount][xcount]);
		}
	}
}

void Transfer()
{
	int xcount, ycount;
	char answer = 0;
	int space = Abs(rand() * RTC_GetTicks()) % 10;
	
	if(connected == True)
	{
		
		if(recieveline > sendline)
		{
			recieveline -= sendline;
			sendline = 0;
		}
		else
		{
			sendline -= recieveline;
			recieveline = 0;
		}
		while(answer != Received)
		{
			Serial_ReadOneByte(&answer);
			Serial_BufferedTransmitOneByte(sendline);
			Sleep((rand() * RTC_GetTicks()) % 2);
		}
		
		if(recieveline > 0)
		{
			for(ycount = 0; ycount < 20 - recieveline; ycount++)
			{
				for(xcount = 0; xcount < 10; xcount++)
				{
					globalmap[ycount][xcount] = globalmap[ycount + recieveline][xcount];
				}
			}
			
			for(ycount = 19; ycount > 19 - recieveline; ycount --)
			{
				for(xcount = 0; xcount < 10; xcount ++)
				{
					globalmap[ycount][xcount] = (xcount == space) ? 0 : 1;
				}
			}
		}
	}
}

void Recieve(void)
{
	int answer = 0; 
	if(Serial_ReadOneByte(&recieveadd) != 0)
	{
		recieveadd = 0;
	}
	recieveline += recieveadd;
	while(Serial_ReadOneByte(&recieveadd) == 0)
	{
		Serial_BufferedTransmitOneByte(Received);
		Sleep((rand() * RTC_GetTicks()) % 2);
	}
	Sleep((rand() * RTC_GetTicks()) % 2);
	if(Serial_ReadOneByte(&recieveadd) != 0)
	{
		recieveadd = 0;
	}
	recieveline += recieveadd;
	while(Serial_ReadOneByte(&recieveadd) == 0)
	{
		Serial_BufferedTransmitOneByte(Received);
		Sleep((rand() * RTC_GetTicks()) % 2);
	}
}

void Disconnect()
{
	Serial_Close(1);
	connected = False;
	Timer_Uninstall(8);
}

void Connect()
{
	unsigned char connstart = Startconnection;
	unsigned char answer;
	int nobock = False;
	char open_mode[6];
	unsigned char key;
	   
	 PopUpWin(1);
	 locate( 5 , 4);
	 Print((unsigned char*)"Connecting...");
	 PutDisp_DD();
	   
	open_mode[ 0 ] = 0; // always 0
	open_mode[ 1 ] = 9; // 0=300, 1=600, 2=1200, 3=2400, 4=4800, 5=9600, 6=19200, 7=38400, 8=57600, 9=115200 baud
	open_mode[ 2 ] = 2; // parity: 0=no; 1=odd; 2=even
	open_mode[ 3 ] = 0; // datalength: 0=8 bit; 1=7 bit
	open_mode[ 4 ] = 0; // stop bits: 0=one; 1=two
	open_mode[ 5 ] = 0; // always 0
	   
	Serial_Open( &open_mode );
	
	while(answer != Startconnection && nobock == False)
	{
		Serial_ReadOneByte(&answer);
		Serial_BufferedTransmitOneByte(connstart);
		Sleep((rand() * RTC_GetTicks()) % 2);
		if(PRGM_GetKey() == 47)
		{
			nobock = True;
		}
	}
	
	answer = 0;
	
	while(answer == Startconnection && nobock == False)
	{
		Serial_ReadOneByte(&answer);
		Sleep((rand() * RTC_GetTicks()) % 2);
		if(PRGM_GetKey() == 47)
		{
			nobock = True;
		}
	}
	connected = (nobock == True)? False : True;
	
	AllClr_VRAM();
	PopUpWin(1);
	locate( 7 , 4);
	if(connected == True)
	{
		Print((unsigned char*)"Ready!");
		Timer_Install(8, &Recieve, 3);
		Timer_Start(8);
	}
	else
	{
		Print((unsigned char*)"Failed!");
		Disconnect();
	}
	Sleep(500);
	GetKey(&key);
}

void CreateBlock()
{
	block = nextblock;
	canswap = True;
	nextblock = 0;
	while(nextblock < 1 || nextblock > 7)
	{ 
		nextblock = Abs((rand() * RTC_GetTicks()) % 7 + 1);
	}
	xpos = 7;
	ypos = 2;
	dir = 1;
	left = True;
	right = True;
	cantrans = True;
}

void SwapPieces()
{
	int save;
	if(canswap == True)
	{
		save = hold;
		hold = block;
		block = save;
		xpos = 7;
		ypos = 0;
		dir = 0;
		if(save == 0)
		{
			CreateBlock();
		} 
	}
	canswap = False;
}

int DrawSprite()
{
	int fit = True;
	
	int xcount, ycount;
	right = True;
	left = True;	 
	
	for(xcount = 0; xcount < 10; xcount ++)
	{
		for(ycount = 0; ycount < 21; ycount ++)
		{
				changemap[ycount][xcount] = 0;
		}
	}

	switch(block)
	{
		case 1: 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
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
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
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
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
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
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
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
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
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
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
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
	char strnextblock[4] = "NEXT";
	char strhold[4] = "HOLD";
	char strpoints[3] = "PTS";
	char strlv[2] = "LV";
	char levelz[2];
	unsigned char pointz[6] = {0};

	int xcount, ycount;
		
	clrbuffer(pScreen);	
	
	levelz[0] = (int)(level / 10) % 10;
	levelz[1] = (int)(level / 10) % 100 / 10;
	
	drawNumber(pScreen, levelz[0], 120, 47);
	drawNumber(pScreen, levelz[1], 120, 51);
	
	drawString(pScreen, strlv, 2, 120, 59);
	
	pointz[5] = points / 100000;
	pointz[4] = points % 100000 / 10000;
	pointz[3] = points % 10000 / 1000;
	pointz[2] = points % 1000 / 100;
	pointz[1] = points % 100 / 10;
	pointz[0] = points % 10;

	drawNumber(pScreen, pointz[0], 112, 43);
	drawNumber(pScreen, pointz[1], 112, 47);
	drawNumber(pScreen, pointz[2], 112, 51);
	drawNumber(pScreen, pointz[3], 112, 55);
	drawNumber(pScreen, pointz[4], 112, 59);
	drawNumber(pScreen, pointz[5], 112, 63);
	
	drawString(pScreen, strpoints, 3, 105, 50);
	
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

	drawString(pScreen, &strnextblock, 4, 105, 5);
	
	switch(nextblock)
	{
		case 1 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockO[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 0 + xcount, 0);
					}
				}
			}
			break;
			
		case 2 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockI[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 0 + xcount, 0);
					}
				}
			}
			break;
			
		case 3 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockT[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 0 + xcount, 0);
					}
				}
			}
			break;
			
		case 4 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockJ[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 0 + xcount, 0);
					}
				}
			}
			break;

		case 5 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockL[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 1 + xcount, 4);
					}
				}
			}
			break;
			
		case 6 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockZ[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 0 + xcount, 0);
					}
				}
			}
			break;
			
		case 7 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockS[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 0 + xcount, 0);
					}
				}
			}
			break;
	}
	
	drawString(pScreen, &strhold, 4, 105, 27);
	
	switch(hold)
	{
		case 1 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockO[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 5 + xcount, 0);
					}
				}
			}
			break;
			
		case 2 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockI[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 5 + xcount, 1);
					}
				}
			}
			break;
			
		case 3 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockT[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 5 + xcount, 2);
					}
				}
			}
			break;
			
		case 4 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockJ[1][ycount][xcount] == 1)
					{
						fillbox(&pScreen, 22 + ycount , 5 + xcount, 3);
					}
				}
			}
			break;

		case 5 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockL[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 5 + xcount, 4);
					}
				}
			}
			break;
			
		case 6 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockZ[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 5 + xcount, 5);
					}
				}
			}
			break;
			
		case 7 : 
			for(xcount = 0; xcount < 4; xcount ++)
			{
				for(ycount = 0; ycount < 4; ycount ++)
				{
					if(blockS[1][ycount][xcount] == 1)
					{
						fillbox(pScreen, 22 + ycount , 5 + xcount, 6);
					}
				}
			}
			break;
	}	
}

void Rotate()
{
	dir += 1;
	if(dir == 4)
	{
		dir = 0;
	}
	if(DrawSprite() == False)	
	{
		dir = (dir == 0)? 3 : dir - 1;
	}
}

void MoveBlock()
{
	switch (nextdir)
	{
		case Left   : if(left == True) {xpos ++;} nextdir = Down;break;
		case Right: if(right == True){xpos --;}nextdir = Down;break;
		case Rot   : Rotate();nextdir = Down;break;
		case Swap: SwapPieces();nextdir = Down;break;
	}
	slowyposdown +=  0.25;
	ypos += ((int)slowyposdown == slowyposdown || nextdir == Fast)? 1 : 0; 
	
	if(nextdir == Fast && (int)slowyposdown == slowyposdown)
	{
		points += 1;
	}
}

void CheckPoints()
{
	int typecount = 0;
	int oldlevel;
	int xcount, ycount;
	int ycount2, ycount3;
	int fulllinecount = 0;
	
	for(xcount = 0; xcount < 10; xcount ++)
	{
		if(globalmap[0][xcount] != 0)
		{
			exit = True;
		}
	}
		
	for(ycount = 0; ycount < 20; ycount ++)
	{
		fulllinecount = 0;
		for(xcount = 0; xcount < 10; xcount ++)
		{
			if(globalmap[ycount][xcount] != 0)
			{
				fulllinecount ++;
			}
			else
			{
				break;
			}
		}
		
		if(fulllinecount == 10)
		{
			typecount ++;
			
			for(ycount2 = ycount; ycount2 > 0; ycount2 --)
			{
				for(xcount = 0; xcount < 10; xcount ++)
				{
					globalmap[ycount2][xcount] = globalmap[ycount2 - 1][xcount];
				}
			}
			line(pScreen, 0 , 0 , 81 , 0, 1);
			line(pScreen,  0 , 0 , 0 , 41, 1);
			line(pScreen,  81 , 0 , 81 , 41, 1);
			line(pScreen,  81 , 41 , 0 , 41, 1);
		
			for(ycount3 = 0; ycount3 < 20; ycount3 ++)
			{
				for(xcount = 0; xcount < 10; xcount ++)
				{
					if(globalmap[ycount3][xcount] != 0 || changemap[ycount3][xcount] != 0)
					{
						fillbox(pScreen, ycount3, xcount, (globalmap[ycount3][xcount] != 0)? globalmap[ycount3][xcount] - 1 : changemap[ycount3][xcount] - 1);
					}
				}
			}
		}
	}
	if(typecount > 0)
	{
		points += (typecount == 1)? 40 * (level + 1): (typecount == 2)? 100 * level : (typecount == 3)? 300 *(level + 1): (typecount == 4)? 1200 * (level + 1) : 0;
		sendline = (typecount == 4)? 4 : typecount - 1;
		oldlevel = level;
		level += typecount;
		if((int)(level / 10) != (int)(oldlevel / 10))
		{
			levelchanged = True;
		}
	}
	Transfer();

}

void ReadKey()
{
	int key;
	
	key = PRGM_GetKey();
		
	nextdir =Down;
		
 	switch(key)
	{
		case /*KEY_CTRL_LEFT*/  38  :  nextdir = Rot;   break;
		case /*KEY_CTRL_DOWN*/37:  nextdir = Left  ;break;
		case /*KEY_CTRL_UP*/       28:  nextdir = Right;break;
		case /*KEY_CTRL_RIGHT*/27:  nextdir = Fast;   break;
		case /*KEY_CTRL_EXIT*/    47:  exit = True;       break;
		case /*KEY_CHAR_TAN*/  26:  nextdir = Swap;break;
	}
}

void Main(void)
{
	cantrans = False;
	ready = False;
	MoveBlock();
	DrawBlock();
	if(cantrans == True)
	{
		CheckPoints();
		DrawBlock();
	}
	ready = True;
}

void CallMain(void)
{
	ReadKey();
	if(nextdir == Fast && onoff == False)
	{
		while(ready == False){}
		onoff = True;
		Timer_Uninstall(7);
		Timer_Install(7, &Main, 50);
		Timer_Start(7);
		if(nextdir == Down)
		{
			nextdir = Fast;
		}
	}
	else
	{
		if(nextdir != Fast && onoff == True || levelchanged == True)
		{
			while(ready == False){}
			onoff = False;
			Timer_Uninstall(7);
			if(level < 100)
			{
				Timer_Install(7, &Main, 200 - (int)(level / 10) * 20);
			}
			else
			{
				 Timer_Install(7, &Main, 0);
			}
			Timer_Start(7);
			levelchanged = False;
		}
	}
}


int AddIn_main(int isAppli, unsigned short OptionNum)
{
	char *screen[3072];
		
	level = 0;
	
	pScreen = &screen;

	nextblock = (rand() * RTC_GetTicks()) % 7 + 1;
	CreateBlock();
	
	ready = True;
	
	Connect();
	
	AllClr_DDVRAM();
	
	Timer_Install(6,&CallMain, 50);
	Timer_Start(6);
		
	while(exit == False)
	{
		RestGray();
	}
	Timer_Uninstall(6);
	Timer_Uninstall(7);
	Timer_Uninstall(8);
	Disconnect();
}

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{return INIT_ADDIN_APPLICATION(isAppli, OptionNum);}
#pragma section
