/*
 * Matris (C) by Casimo, summer 2012
 * licensed as GPLv3+
 */

#include "stdio.h"
#include "key.h"
#include "draw.h"
#include "global.h"

short*APP_EnableRestart()
{
	unsigned int ea;
	unsigned int j;
	short*pEnableRestartFlag;
	ea = *(unsigned int*)0x8001007C;
	ea += 0x0490*4;
	if ( ea < 0x8001007C ) return 0;
	if ( ea > 0x81000000 ) return 0;
	ea = *(unsigned int*)( ea );
	if ( ea < 0x8001007C ) return 0;
	if ( ea > 0x81000000 ) return 0;
	j = *(unsigned char*)( ea + 1 );
	j *= 4;
	ea = ( ea + j + 4 ) & 0xFFFFFFFC;
	if ( ea < 0x8001007C ) return 0;
	if ( ea > 0x81000000 ) return 0;
	pEnableRestartFlag = (short*)(*( unsigned int*)( ea ) + 8 );
	if ( ( (unsigned int)pEnableRestartFlag & 0xFF000000 ) != 0x88000000 ) return 0;
	if ( pEnableRestartFlag ) *pEnableRestartFlag = 1;
	return pEnableRestartFlag;
}

//restart addin after return 1

static unsigned int seed = 123456789;
unsigned int kiss()
{
	// interner Zustand
	static unsigned int y = 362436000;
	static unsigned int z = 521288629;
	static unsigned int c = 7654321;
 
	unsigned long t;
	
	// Linearer Kongruenzgenerator
	seed = 69069 * seed + 12345;
	
	// Xorshift
	y ^= y << 13;
	y ^= y >> 17;
	y ^= y << 5;
	
	// Multiply-with-carry
	t = (unsigned long)698769069 * z + c;
	c = t >> 32;
	z = (unsigned int) t;
	 
	return seed + y + z;
}

void seedkiss()
{
	seed = RTC_GetTicks();
}

//kiss is a random number generator

char *Disp_GetVRAMPtr(void);
void DrawLineVRAM(int x1, int y1, int x2, int y2);
void AllClr_DD(void);
void AllClr_VRAM(void);
void AllClr_DDVRAM(void);
void locate(int x, int y);
void Print(const unsigned char *str);
int Timer_Install(int TimerID, void (*handler)(void), int delay);
int Timer_Uninstall(int TimerID);
int Timer_Start(int TimerID);
void App_RUN_MAT();

//some prototypes

char swit;

char pause = False;
//pause the game

char quit = False;

char levelchanged = False;
//has the level changed during two actions?

char exit = False;

char ready;

char onoff = True;
//used for timers

char canswap;
//brick already swapped?

char slowyposdown = 0;

char connected;
//multiplayer activated?

unsigned char receiveline = 0;
unsigned char sendline     = 0;
//what to send or receive?

char cantrans = False;
//allowed to transfer?

//taken from MonochromeLib by PierrotLL

void drawbuffer(char *buffer)
{
	char *LCD_register_selector = (char*)0xB4000000, *LCD_data_register = (char*)0xB4010000;
	int i, j;
	for(i=0 ; i<64 ; i++)
	{
		*LCD_register_selector = 4;
		*LCD_data_register = i|192;
		*LCD_register_selector = 4;
		*LCD_data_register = 0;
		*LCD_register_selector = 7;
		for(j=0 ; j<16 ; j++) *LCD_data_register = *buffer++;
	}
}

void RestGray(void)
{
	drawbuffer(pScreen);
	drawbuffer(pScreen + 1024);
	drawbuffer(pScreen + 2048);
}


void Disconnect()
{
	Serial_Close(1);
	connected = False;
}

void TeacherMode()
{
	Timer_Uninstall(6);
	Timer_Uninstall(7);
	Disconnect();

	App_RUN_MAT();
}

int round(float num)
{
	return (int) (num + 0.5);
}

void clrbuffer(char *buffer)
{
	memset(buffer, 0, 3072);
}


void CreateBlock()
{
	block = nextblock;
	canswap = True;
	nextblock = (kiss() % 7) + 1;
	xpos = 7;
	ypos = 3;
	dir = 0;
	left = True;
	right = True;
	cantrans = True;
}

void ResetValues()
{
	int x,y;
	
	char text[30];

	memset(&changemap, 0, 210);
	memset(&globalmap, 0, 210);
	
	ypos = 3;
	xpos = 0;
	block = 0;
	onoff = True;
	levelchanged = False;
	right = True;
	left = True;
	hold = 0;
	exit = False;
	nextdir = Down;
	nextblock = (kiss() % 7) + 1;
	CreateBlock();
	clrbuffer(pScreen);
	Timer_Uninstall(6);
	Timer_Uninstall(7);
	ready = True;
	connected = False;
	
	AllClr_DDVRAM();
	
	PopUpWin(2);
	sprintf(&text, "Points: %d", points);
	locate(3, 3);
	Print(&text);
	sprintf(&text, "Level: %d", level);
	locate(3, 4);
	Print(&text);
	PutDisp_DD();
	
	points = 0;
	level = 0;

	while(!KeyDown(31));
}

void Transfer()
{
	int xcount, ycount;
	char space = kiss() % 10; //where is the hole?
	
	if(connected == True)
	{
		if(receiveline > sendline)
		{
			//if there are lines arriving, but there can be send lines back, erase the lines, for example: receiving 3 lines, sending 1 -> 0 are send, 2 are received
			receiveline -= sendline;
			sendline = 0;
		}
		else
		{
			sendline -= receiveline;
			receiveline = 0;
		}
		if(sendline > 0)
		{
			while(Serial_BufferedTransmitOneByte(sendline) != 0);
			//send the number of lines
			sendline = 0;
		}
		if(receiveline > 0)
		{
			for(ycount = 0; ycount < 20 - receiveline; ycount++)
			{
				for(xcount = 0; xcount < 10; xcount++)
				{
					globalmap[ycount][xcount] = globalmap[ycount + receiveline][xcount];
					//move all lines upwards
				}
			}
			
			for(ycount = 19; ycount > 19 - receiveline; ycount --)
			{
				for(xcount = 0; xcount < 10; xcount ++)
				{
					globalmap[ycount][xcount] = (xcount == space) ? 0 : (kiss() % 7) + 1;
					//add lines at the bottom
				}
			}
			receiveline = 0;
		}
	}
}

void receive(void)
{
	unsigned char receiveadd = 0;
	
	if(Serial_ReadOneByte(&receiveadd) != 0)
	{
		receiveadd = 0;
	}
	
	receiveline += (receiveadd > 20)?0:receiveadd;
}

void Connect()
{
	unsigned char connstart = 111;
	unsigned char answer;
	char open_mode[6];
		   
	 PopUpWin(1);
	 locate( 5 , 4);
	 Print("Connecting...");
	 PutDisp_DD();
	   
	open_mode[ 0 ] = 0; // always 0
	open_mode[ 1 ] = 9; // 0=300, 1=600, 2=1200, 3=2400, 4=4800, 5=9600, 6=19200, 7=38400, 8=57600, 9=115200 baud
	open_mode[ 2 ] = 2; // parity: 0=no; 1=odd; 2=even
	open_mode[ 3 ] = 0; // datalength: 0=8 bit; 1=7 bit
	open_mode[ 4 ] = 0; // stop bits: 0=one; 1=two
	open_mode[ 5 ] = 0; // always 0
	   
	Serial_Open( &open_mode );
	
	do
	{
		Serial_BufferedTransmitOneByte(connstart);
		Serial_ReadOneByte(&answer);
	}while(answer != connstart && !KeyDown(47));
	
	if(!KeyDown(47))connected = True;
	
	if(connected != True)
	{
		Disconnect();
	}
}

void SwapPieces()
{
	char save;
	if(canswap == True)
	{
		save = block;
		if(hold == 0)
		{
			block = nextblock;
			nextblock = (kiss() % 7) + 1;
		}
		else
		{
			block = hold;
		}
		hold = save;
		xpos = 7;
		ypos = 3;
		dir = 0;
	}
	canswap = False;
}

void Rotate()
{
	if(++dir == 4)
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
	
	if(nextdir == Fast && (int)slowyposdown == slowyposdown)
	{
		points += 1;
	}
}

void CheckPoints()
{
	char typecount = 0;
	char oldlevel;
	int xcount, ycount;
	int ycount2, ycount3;
	char fulllinecount = 0;
	
	for(xcount = 0; xcount < 10; xcount ++)
	{
		if(globalmap[0][xcount] != 0)
		{
			exit = True;
			//game over
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
			
			//1: single, 2: double, 3: triple line, 4: tetris (maximum)
			
			for(ycount2 = ycount; ycount2 > 0; ycount2 --)
			{
				for(xcount = 0; xcount < 10; xcount ++)
				{
					globalmap[ycount2][xcount] = globalmap[ycount2 - 1][xcount];
					//move averything
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
		int blink;
		
		points += (typecount == 1)? 40 * (level + 1): (typecount == 2)? 100 * level : (typecount == 3)? 300 *(level + 1): (typecount == 4)? 1200 * (level + 1) : 0;
		//increment points
		
		sendline = (typecount == 4)? 4 : typecount - 1;
		//send a line, if not tetris send lines - 1
		oldlevel = level;
		level += typecount;
		if((int)(level / 10) != (int)(oldlevel / 10) && level < 200)
		{
			levelchanged = True;
			//level = round cleared lines / 10
		}
	}
	Transfer();
}

void ReadKey()
{
	nextdir =Down;
		
	if(KeyDown(38)) nextdir = Rot;
	if(KeyDown(37))  nextdir = Left  ;
	if(KeyDown(28))  nextdir = Right;
	if(KeyDown(27))  nextdir = Fast;   
	if(KeyDown(47))  exit = True; 
	if(KeyDown(26))  nextdir = Swap;
	if(KeyDown(31)) TeacherMode();
	if(KeyDown(39)) {pause = (pause == True)? False : True; while(KeyDown(39));}
}

void Main(void)
{
	cantrans = False;
	ready = False;
	if(pause == False)
	{
		receive();
		ypos += (slowyposdown % 10 == 0 || nextdir == Fast)? 1 : 0; 
		if(nextdir == Left || nextdir == Right || nextdir == Fast || nextdir == Rot || slowyposdown % 10 == 0)
		{
			MoveBlock();
			DrawBlock();
			if(cantrans == True)
			{
				CheckPoints();
				DrawBlock();
			}
		}
		slowyposdown++;
	}
	ready = True;
	drawNumber(pScreen, connected, 0, 62);
	drawNumber(pScreen + 1024, connected, 0, 62);
	drawNumber(pScreen + 2048, connected, 0, 62);
}

void CallMain(void)
{
	ReadKey();
	if(nextdir == Fast && onoff == False)
	{
		while(ready == False);
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
			while(ready == False);
			onoff = False;
			Timer_Uninstall(7);
			
			if(level < 20)
			{
				Timer_Install(7, &Main, 100 - level * 5);
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

//the code above is not good, CallMain handles the keys and sets the timer of Main

void ReadMenuKeys()
{
	if(KeyDown(31)) TeacherMode();
	if(KeyDown(47)) quit = True;
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
	char c[] = {0xE6, 0x9B, 0};	
	char mode = 0;
	
	level = 0;
	
	pScreen = (char*)malloc(3072);

	nextblock = (kiss() % 7) + 1;
	CreateBlock();
	
	ready = True;
	
	APP_EnableRestart();//thx to SimonLothar
	
	seedkiss();//seed random numbers
	
	while(mode != 3)
	{	
		quit = False;
		Timer_Install(10, &ReadMenuKeys, 200);
		Timer_Start(10);
		while(!KeyDown(31) && mode != 3)
		{
			AllClr_VRAM();
		
			locate( 6, 3 );
			Print(&"Singleplayer");
			locate( 6, 4 );
			Print(&"Multiplayer");
			locate( 2, 8);
			Print(&"Copyright by Casimo");
	
			locate( 4, 3 + mode);
			PrintC(&c);
			
			PutDisp_DD();
			
			if(KeyDown(37))
			{
				mode += (mode == 1)? -1 : 1;
				while(KeyDown(37));
			}
			else
			{
				if(KeyDown(28))
				{
					mode -= (mode == 0)? -1 : 1;
					while(KeyDown(28));
				}
				else
				{
					if(KeyDown(47))
					{
						mode = 3;
						while(KeyDown(47));
					}
				}
				
			}
		}
		
		Timer_Uninstall(10);
							
		if(mode != 3)
		{
			if(mode == 1)
			{
				Serial_ClearReceiveBuffer();
				Connect();
			}
			
			Timer_Install(6,&CallMain, 150);
			Timer_Start(6);
			
			while(exit == False){RestGray();}
			
			ResetValues();
			
			while(KeyDown(47));
			while(KeyDown(31));
		}
	}
	Timer_Uninstall(6);
	Timer_Uninstall(7);
	Disconnect();
	free(pScreen);
}

//I'm sure there are unused variables

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{return INIT_ADDIN_APPLICATION(isAppli, OptionNum);}
#pragma section
