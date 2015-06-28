/* 
by David P. Discher - 2015

*/

#include <stdio.h>  // printf
#include <uuid.h>   // uuid
#include <stdlib.h> // free
#include <string.h>
#include <netinet/in.h>
#include "dpdChatFabric.h"

int main(int argc, char**argv)
{

	uuid_t uuid;
	uint32_t status,i,ni;
	char *str;
	unsigned char b[16];
	unsigned char h,l;
	unsigned char paddingLength;

	for (i=0; i<sizeof(b); i++) {
		b[i]=0;
	}

	uuid_create_nil(&uuid,  &status);		
	uuid_to_string(&uuid, &str, &status);		
	printf ( "UUID-TLD: %s\n", str );	
	free(str);

	uuid_create(&uuid,  &status);		
	uuid_to_string(&uuid, &str, &status);		
	printf ( "UUID-ITM: %s\n\n", str );	
	free(str);
/*
	for (i=0; i<1024; i++) {
		ni = htonl(i);
		memcpy(&b, &ni, 4);
		printf ("%02x %02x %02x %02x\n", b[0],b[1],b[2],b[3] );
	}
*/


	for (i=0; i<sizeof(b); i++) {
		b[i]=0;
	}

	h =  arc4random_uniform(15) + 1;
	l = 16 - h;
	arc4random_buf(&b, 16);
	printf ( "====> %02d %02d \n", h, l );
	printf ( "====> %02x %02x \n", h, l );

	paddingLength = (h << 4) | l;

	printf ( "====> %02x\n", paddingLength );

}


