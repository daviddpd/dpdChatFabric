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

extern void curve25519_donna(u8 *output, const u8 *secret, const u8 *bp);

void pp(unsigned char * x) {
	int i;
	for (i=0; i<32; i++) {
		printf ( "%02x", x[i] );
	}
	printf ("\n");

}

int main(int argc, char**argv)
{
	unsigned char shared_key[crypto_box_BEFORENMBYTES], shared_key2[crypto_box_BEFORENMBYTES];
	const unsigned char basepoint[32] = {9};
	int i = 0;


// The quick brown fox jumps over the lazy dog.
#define MESSAGE (const unsigned char *) "0123456789 The quick brown fox jumps over the lazy dog. _ 60 ______ 70 ______ 80 ______ 90 ______100"
#define MESSAGE_LEN 100
#define ADDITIONAL_DATA (const unsigned char *) "123456"
#define ADDITIONAL_DATA_LEN 6

unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES];
unsigned char key[crypto_aead_chacha20poly1305_KEYBYTES];
unsigned char ciphertext[MESSAGE_LEN + crypto_aead_chacha20poly1305_ABYTES];
unsigned long long ciphertext_len;

unsigned char decrypted[MESSAGE_LEN+1];
unsigned long long decrypted_len;


unsigned char mypublic[crypto_box_PUBLICKEYBYTES];
unsigned char mysecret[crypto_box_SECRETKEYBYTES];

unsigned char hispublic[crypto_box_PUBLICKEYBYTES];
unsigned char hissecret[crypto_box_SECRETKEYBYTES];


/*

Create "My" public/private key pair using libsodium using Curve25519-Poly1305

	Key exchange: Curve25519
	Encryption: XSalsa20 stream cipher
	Authentication: Poly1305 MAC

	https://download.libsodium.org/doc/public-key_cryptography/authenticated_encryption.html

*/

crypto_box_keypair((unsigned char *)&mypublic, (unsigned char *)&mysecret);
printf ( "mysecret: ");
pp ( (unsigned char *)&mysecret );
printf ( "mypublic: ");
pp ( (unsigned char *)&mypublic );

/*
	Create "His" public/private key pair using libsodium
*/

crypto_box_keypair((unsigned char *)&hispublic, (unsigned char *)&hissecret);
printf ( "hissecret: ");
pp ( (unsigned char *)&hissecret );
printf ( "hispublic: ");
pp ( (unsigned char *)&hispublic );


/*
	Calculate a shared-secret to use 

	http://doc.libsodium.org/public-key_cryptography/authenticated_encryption.html#precalculation-interface

	Key exchange: Curve25519
	Encryption: XSalsa20 stream cipher
	Authentication: Poly1305 MAC

*/

crypto_box_beforenm((unsigned char *)&shared_key,  (unsigned char *)&hispublic, (unsigned char *)&mysecret);
printf ( "shared_key : ");
pp ( (unsigned char *)&shared_key );

crypto_box_beforenm((unsigned char *)&shared_key2, (unsigned char *)&mypublic,  (unsigned char *)&hissecret);
printf ( "shared_key2: ");
pp ( (unsigned char *)&shared_key2 );
                        
/*
   Do the aead_chacha20poly1305 encrypt with shared_key 1
   http://doc.libsodium.org/secret-key_cryptography/aead.html

*/

crypto_aead_chacha20poly1305_encrypt(ciphertext, &ciphertext_len,
                                     MESSAGE, MESSAGE_LEN,
                                     ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
                                     NULL, nonce, shared_key);
                                     
                                     
/*
   Do the aead_chacha20poly1305 decrypt with shared_key 2 
   http://doc.libsodium.org/secret-key_cryptography/aead.html

*/

if (crypto_aead_chacha20poly1305_decrypt(decrypted, &decrypted_len,
                                         NULL,
                                         ciphertext, ciphertext_len,
                                         ADDITIONAL_DATA,
                                         ADDITIONAL_DATA_LEN,
                                         nonce, shared_key2) == 0) 
    { 
    
    decrypted[100] = 0;
    
    	printf ( "Message good! \n" );
    	printf ( "CipherText[%d]: ", ciphertext_len );
    	for (i=0; i<ciphertext_len; i++) {
    		printf ("%02x", ciphertext[i] );
    	}
    	printf("\n");
    	printf ( "Original  Message[%d]: %s\n", MESSAGE_LEN, MESSAGE );
    	printf ( "Decrypted Message[%llu]: =%s=\n", decrypted_len, decrypted );
//    	printf ( "Decrypted Message[%llu]: =%02x=\n", decrypted_len, decrypted[100] );
    	
    	
    	
    
    } else {
    	printf ( "Message bad! \n" );
    
    }
	

}