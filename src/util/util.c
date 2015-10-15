#include "util.h"
#include "cfstructs.h"
#include "assert.h"

int _GLOBAL_DEBUG;

void CP_ICACHE_FLASH_ATTR util_debug_bin2hex(char* label, unsigned char * x, int len, char* file, const char* func, int line ){

	int i;
	int z=0;
    int l = (len/4) + len*2;
    if (len % 4 > 0) { ++l; }

	char *_utilbuffer = (char *)malloc( (l+1) * sizeof(char) );
    bzero(_utilbuffer, l+1);
    
    
	for (i=0; i<len; i++) {
		sprintf(_utilbuffer+z, "%02x", x[i]);
		z+=2;

		if ( (i>0) && ( (i+1)%4 == 0 ) ) {
			sprintf (_utilbuffer+z, " " );
			z++;
		}

	}

	printf("[DEBUG][%s:%s:%d] %20s: %s\n", file, func, line, label, _utilbuffer );
    free(_utilbuffer);
}



void CP_ICACHE_FLASH_ATTR util_bin2hex (char *cd, char* label, unsigned char * x, int len ){
	int i;
	int z=0;
    int l = (len/4) + len*2;
    if (len % 4 > 0) { ++l; }

    char *_utilbuffer = (char *)malloc( (l+1) * sizeof(char) );
    bzero(_utilbuffer, l+1);
    
	for (i=0; i<len; i++) {
		sprintf(_utilbuffer+z, "%02x", x[i]);
		z+=2;

		if ( (i>0) && ( (i+1)%4 == 0 ) ) {
			sprintf (_utilbuffer+z, " " );
			z++;
		}

	}
	printf ( "%2s %24s: %42s\n", cd, label, _utilbuffer);
    free(_utilbuffer);

}