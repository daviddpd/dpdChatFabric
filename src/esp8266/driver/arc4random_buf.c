#include "arc4random_buf.h"

void CP_ICACHE_FLASH_ATTR arc4random_buf(unsigned char *b, int len )
{
	int i;
	for (i=0; i<len; i++ ) {
		b[i] = (unsigned char)os_random() % 255;	
	}
}
