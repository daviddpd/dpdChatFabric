
#include <stdio.h>
#include <sys/types.h> // kqueue / kevent 
#include <stdlib.h> // exit
#include <netinet/in.h>

#define crypto_secretbox_NONCEBYTES 8U
void
print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { printf (" "); }
	}
	printf ("\n");

}

int main(int argc, char**argv)
{
	uint32_t u,l;
	unsigned char mynonce[crypto_secretbox_NONCEBYTES];
	arc4random_buf(&(mynonce), crypto_secretbox_NONCEBYTES);
	
	while (1) {
		print_bin2hex( &(mynonce), crypto_secretbox_NONCEBYTES );
		memcpy ( &u, &(mynonce), 4);
		memcpy ( &l, &(mynonce[4]), 4);
		u = htonl(u);
		l = htonl(l);
		l++;
		u = ntohl(u);
		l = ntohl(l);		
		memcpy (  &(mynonce),    &u, 4);
		memcpy (  &(mynonce[4]), &l, 4);
	}	
}
