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

unsigned char ciphertext2[MESSAGE_LEN + crypto_box_SEALBYTES];
unsigned char ciphertext3[MESSAGE_LEN + crypto_secretbox_MACBYTES];


unsigned char decrypted[MESSAGE_LEN+1];
unsigned long long decrypted_len;


unsigned char mypublic[crypto_box_PUBLICKEYBYTES];
unsigned char mysecret[crypto_box_SECRETKEYBYTES];

unsigned char hispublic[crypto_box_PUBLICKEYBYTES];
unsigned char hissecret[crypto_box_SECRETKEYBYTES];

// unsigned char nonce2[crypto_secretbox_NONCEBYTES];

int msg1len = 1162;
//arc4random_buf(&(nonce2), crypto_secretbox_NONCEBYTES);
unsigned char *msg1_hex = (unsigned char*) "b7b9a33b97d5697c5659f33dc5723b0c6778d012ef8ebcabd36973d0c32db7adc982879cae1b65e6c7e86cd7f3790abd76fa3320322a2a86123ce8d5315b1edba981ee87da535677e2d49ab1aedd24e5cb07da136cbb469da2422d41eab6d70e5993be276021d41352cdc93c732f8897fa463d556fa835dac8d0126641f77649574dd13a032115e7793cd90fdacac13b764c5963b9a021074267ef45bc7ed816c153a3b7a7f9d53b98bb12a1888d2499eb1102db8e4c02f9316ea1fd3fc347cf3fc5b45751e02640f4b08fcdbdbf4bd7e1dc72c2344dfce4aaa8837e6713bd5545426047a9baff51b68b15a2bc72cf275a9cd90bf56fa0b5fa3dc1a6bcb93a5b115bc7456f425f7ca562dbbe2a71ef7f2f8ff7f1439e94cc238c60e20796d6e148e2f3754b253bd1774f24820dbc09678b52932324526ba80ac087d28d21d0420794612116d2b1c9d787ba6c28cf11964f97a148df78e82f92a5caf7561c95d91fcaed3ce044d83ed55c4af3286d3fb6d44cc0f4ed5b7027d190d2c09f4f6a69122fcdb2f9d5d3d865b84abe8cf36e5a51529914cf2d96dbca94f5ddaf293f1cdd7c4373e60b6eebfb4af1e25f888c85f920712952775d91e98ef034493bf5688fee92c584a67b81d2904dac88783ee934b08b7361714ed59455cb1f952228deb78292caaa05cc90f7fd4991ffc42f15b1a6e92191d8fae59ef37988850259e4e383952866dc37e2076dec9a9241fe091a3228325f4174da26a3eed8e020b1e2501a7ae918d8280fc5e5c8432e5a8e1d4a59e7fd345ee287aa7d92dae2f693fb0000000000";
unsigned char *nonce2_hex = (unsigned char*) "f92bb7c74cb377ebcc0776986f594bf2cc1c1359ba1ffc70";
unsigned char *hispublickey_hex  = (unsigned char*) "a1a3b1385509e56449269f9d995a7d2d6bc1d7edd3b0d404bac8959d177fea4f";
unsigned char *mypublickey_hex =(unsigned char*)"157b15dafa9fc6cf96338a996ea90b39418ae1b5c2ceaace2a2d4767d23da843";
unsigned char *mysecret_hex =(unsigned char*)"8db561b5e5f839dbdcb44d0fb27c25d6fe20264b4b12fd6b93d54c344e0c9e2c";

unsigned char *msg1;
unsigned char *nonce2;
unsigned char *hispublickey;
unsigned char *mypublickey;
unsigned char *mysecretkey;
unsigned char *decrypted2;
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
                                     NULL, 0,
                                     NULL, nonce, shared_key);
                                     
                                     
/*
   Do the aead_chacha20poly1305 decrypt with shared_key 2 
   http://doc.libsodium.org/secret-key_cryptography/aead.html

*/

if (crypto_aead_chacha20poly1305_decrypt(decrypted, &decrypted_len,
                                         NULL,
                                         ciphertext, ciphertext_len,
                                         NULL,
                                         0,
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


// Boxes :
			printf( " ===========> crypto_box_seal!\n");


		for (i=0; i<MESSAGE_LEN; i++ ){
			decrypted[i] = 0;		
		}

		crypto_box_seal(ciphertext2, MESSAGE, MESSAGE_LEN, &(hispublic));
    	printf ( "CipherText[%d]: ", MESSAGE_LEN+crypto_box_SEALBYTES );
    	for (i=0; i<MESSAGE_LEN+crypto_box_SEALBYTES; i++) {
    		printf ("%02x", ciphertext2[i] );
    	}
    	printf("\n");

		if (crypto_box_seal_open(decrypted, ciphertext2, MESSAGE_LEN+crypto_box_SEALBYTES, (unsigned char *)&hispublic, (unsigned char *)&hissecret) == 0)
		{
	    	printf ( "Original  Message[%d]: %s\n", MESSAGE_LEN, MESSAGE );
    		printf ( "Decrypted Message[%llu]: =%s=\n", decrypted_len, decrypted );
		} else {
			printf( " ===========> decryption failed!\n");
		}


		for (i=0; i<MESSAGE_LEN; i++ ){
			decrypted[i] = 0;		
		}
		
		msg1=calloc(msg1len/2,sizeof(unsigned char));
		chatFabric_hex2int_bytes(msg1_hex, msg1len, msg1, msg1len/2);

		nonce2=calloc(crypto_secretbox_NONCEBYTES,sizeof(unsigned char));
		chatFabric_hex2int_bytes(nonce2_hex, crypto_secretbox_NONCEBYTES*2, nonce2, crypto_secretbox_NONCEBYTES);

		nonce2=calloc(crypto_secretbox_NONCEBYTES,sizeof(unsigned char));
		chatFabric_hex2int_bytes(nonce2_hex, crypto_secretbox_NONCEBYTES*2, nonce2, crypto_secretbox_NONCEBYTES);

		hispublickey=calloc(crypto_box_PUBLICKEYBYTES,sizeof(unsigned char));
		chatFabric_hex2int_bytes(hispublickey_hex, crypto_box_PUBLICKEYBYTES*2, hispublickey, crypto_box_PUBLICKEYBYTES);

		mysecretkey=calloc(crypto_box_SECRETKEYBYTES,sizeof(unsigned char));
		chatFabric_hex2int_bytes(mysecret_hex, crypto_box_SECRETKEYBYTES*2, mysecretkey, crypto_box_SECRETKEYBYTES);

		decrypted2=calloc(msg1len/2 - crypto_secretbox_MACBYTES,sizeof(unsigned char));

/*
		crypto_box_easy(
			ciphertext3, MESSAGE, MESSAGE_LEN,
			nonce2,
			hispublickey,
			mysecretkey
		);*/
		if (crypto_box_open_easy(decrypted2, msg1, msg1len/2, nonce2,
								 hispublickey,  mysecretkey ) == 0) {
	    	printf ( "Original  Message[%d]: %s\n", MESSAGE_LEN, MESSAGE );
    		printf ( "Decrypted Message[%llu]: =%s=\n", decrypted_len, decrypted );
		} else {
			printf( " ===========> decryption failed!\n");
		}

}