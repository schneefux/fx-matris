int OSVersionAsInt(void)
{
	unsigned char mainversion;
	unsigned char minorversion;
	unsigned short release;
	unsigned short build;
	GlibGetOSVersionInfo( &mainversion, &minorversion, &release, &build );
	return ( ( mainversion << 24 ) & 0xFF000000 ) | ( ( minorversion << 16 ) & 0x00FF0000 ) | ( release & 0x0000FFFF );
}

#define isOS2 (OSVersionAsInt() >= 0x02020000)

const unsigned short* keyboardregister = (unsigned short*)0xA44B0000;

unsigned short key[8];

int KeyDown(int keycode)
{
	if(isOS2)
	{
		int row, col, word, bit; 
	
		row = keycode%10; 
		col = keycode/10-1; 
		word = row>>1; 
		bit = col + 8*(row&1); 
		
		memcpy(&key, keyboardregister, 8 * sizeof(unsigned short));
		
		return (0 != (key[word] & 1<<bit)); 
	}
	else
	{
		unsigned char buffer[12];
		KBD_PRGM_GetKey( buffer );
		return ( buffer[1] & 0x0F ) * 10 + ( ( buffer[2]  & 0xF0 )  >> 4 ) == keycode;
	}
}

