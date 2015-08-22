/* 

Copyright (c) 2015, David P. Discher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Compiled with on FreeBSD.  security/libsodium is required.

 cc -o curve25519_aead_chacha20poly1305_sample \
 	curve25519_aead_chacha20poly1305_sample.c \
 	/usr/local/lib/libsodium.a \
 	-I/usr/local/include

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include <sodium.h>

typedef uint8_t u8;


// The quick brown fox jumps over the lazy dog.
#define MESSAGE (const unsigned char *) "0123456789 The quick brown fox jumps over the lazy dog. _ 60 ______ 70 ______ 80 ______ 90 ______100"
#define MESSAGE_LEN 100
#define ADDITIONAL_DATA (const unsigned char *) "123456"
#define ADDITIONAL_DATA_LEN 6

extern void curve25519_donna(u8 *output, const u8 *secret, const u8 *bp);

/*
void 
chatFabric_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght ) {
	char str[5] = { '0', 'x', '0', '0' , 0 };
	int i=0,x=0;
	uint32_t dst_int;
	//str[2] = 0;
//	printf ( "==> chatFabric_hex2int_bytes\n" );
	for ( i = 0; i<hexLength; i=i+2) { 
		if  (hex[i] == 32) {
			continue;
		}
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

void pp(unsigned char * x) {
	int i;
	for (i=0; i<32; i++) {
		printf ( "%02x", x[i] );
	}
	printf ("\n");

}

*/

int main(int argc, char**argv)
{
	unsigned char shared_key[crypto_box_BEFORENMBYTES], shared_key2[crypto_box_BEFORENMBYTES];
	const unsigned char basepoint[32] = {9};
	int i = 0, ret;
	uint32_t p_length =0, e_length=0, ob_length=0, encrypted_envolopeLength=0;
	unsigned long long p_length_encrpyted=0;
	unsigned char *envelope;
	unsigned char *envelope_encrypted;
	unsigned char *payload;
	unsigned char *payload_encrypted;
	unsigned char *decrypted=0;


	unsigned char mypublic[crypto_box_PUBLICKEYBYTES];
	unsigned char mysecret[crypto_box_SECRETKEYBYTES];

	unsigned char hispublic[crypto_box_PUBLICKEYBYTES];
	unsigned char hissecret[crypto_box_SECRETKEYBYTES];


	crypto_box_keypair((unsigned char *)&mypublic, (unsigned char *)&mysecret);
//	printf ( "mysecret: ");
//	pp ( (unsigned char *)&mysecret );
//	printf ( "mypublic: ");
//	pp ( (unsigned char *)&mypublic );

	crypto_box_keypair((unsigned char *)&hispublic, (unsigned char *)&hissecret);
//	printf ( "hissecret: ");
//	pp ( (unsigned char *)&hissecret );
//	printf ( "hispublic: ");
//	pp ( (unsigned char *)&hispublic );

	encrypted_envolopeLength = MESSAGE_LEN + crypto_box_SEALBYTES;

	envelope_encrypted =  calloc(encrypted_envolopeLength,sizeof(unsigned char));
	crypto_box_seal(envelope_encrypted, MESSAGE, MESSAGE_LEN, (unsigned char *)&hispublic);
	decrypted=calloc(encrypted_envolopeLength - crypto_box_SEALBYTES,sizeof(unsigned char));

	ret = crypto_box_seal_open(
		decrypted, 
		envelope_encrypted, encrypted_envolopeLength, 
		(unsigned char *)&hispublic, 
		(unsigned char *)&hissecret 
	);
/*	
	if ( ret == 0)
	{
		printf ( "Envelope decryption successful !\n" ); 
	} else {
		printf ( "Envelope decryption FAILED! !\n" ); 
	}

*/
	payload_encrypted =  calloc(MESSAGE_LEN+crypto_secretbox_MACBYTES,sizeof(unsigned char));
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	arc4random_buf(&nonce, crypto_secretbox_NONCEBYTES);

	crypto_box_easy(
		payload_encrypted, 
		MESSAGE, MESSAGE_LEN,		
		(const unsigned char *)&nonce,
		(unsigned char *)&mypublic, 
		(unsigned char *)&hissecret
	);				

	free(decrypted);
	decrypted=calloc(MESSAGE_LEN,sizeof(unsigned char));	

	ret = crypto_box_open_easy(
			decrypted, 
			payload_encrypted, MESSAGE_LEN+crypto_secretbox_MACBYTES, 
			(const unsigned char *)&nonce,  
			(unsigned char *)&mypublic,
			(unsigned char *)&mysecret
	);
/*	
	if ( ret == 0)
	{
		printf ( "payload decryption successful !" ); 
	} else {
		printf ( "payload decryption FAILED! !\n" ); 
	}

*/
	return 0;
}