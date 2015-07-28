#include "util.h"

#ifdef ESP8266

void CP_ICACHE_FLASH_ATTR
util_print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		os_printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { os_printf (" "); }
	}
	os_printf ("\n");

}

void CP_ICACHE_FLASH_ATTR
util_hexprint ( unsigned char *str, uint32_t len ){
	int i;
	unsigned char p;
	os_printf ( "\n %4s: ", " " );		

	for (i=0; i<len; i++) {
		p = str[i];
		if ( p < 32 ) {
			os_printf (" ");
		} else {
			printf ("%c", p);
		}
		if ( (i > 0 ) && (i % 42) == 0 ) {
			os_printf ( "\n %4s: ", ' ' );		
		}
	}

	os_printf ("\n");

}
void CP_ICACHE_FLASH_ATTR
util_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght ) {
	char str[5] = { '0', 'x', '0', '0' , 0 };
	int i=0,x=0;
	uint32_t dst_int;
	for ( i = 0; i<hexLength; i=i+2) {
		str[2] = hex[i];
		str[3] = hex[i+1];
		if ( x < dstLenght ) {
			os_sprintf (str, "%d", &dst_int );
			dst_int = strtoull(str, NULL, 16);
			dst[x] = (unsigned char)dst_int;
		}
		x++;
	}

}
#else
void CP_ICACHE_FLASH_ATTR
util_print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { printf (" "); }
	}
	printf ("\n");

}

void CP_ICACHE_FLASH_ATTR
util_hexprint ( unsigned char *str, uint32_t len ){
	int i;
	unsigned char p;
	printf ( "\n %4s: ", " " );		

	for (i=0; i<len; i++) {
		p = str[i];
		if ( p < 32 ) {
			printf (" ");
		} else {
			printf ("%c", p);
		}
		if ( (i > 0 ) && (i % 42) == 0 ) {
			printf ( "\n %4c: ", ' ' );		
		}
	}

	printf ("\n");

}

void CP_ICACHE_FLASH_ATTR
util_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght ) {
	char str[5] = { '0', 'x', '0', '0' , 0 };
	int i=0,x=0;
	uint32_t dst_int;
	//str[2] = 0;
//	printf ( "==> chatFabric_hex2int_bytes\n" );
	for ( i = 0; i<hexLength; i=i+2) {
		str[2] = hex[i];
		str[3] = hex[i+1];
		if ( x < dstLenght ) {
			sscanf (str, "%d", &dst_int );
			dst_int = strtoul(str, NULL, 16);
			dst[x] = (unsigned char)dst_int;
		}
		
//		printf ( " === %6s ?= %02x = %4d = ===========\n", str, dst[x], dst_int );
		
		x++;
	}

}

#endif
