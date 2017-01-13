#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include "salsa20.h"
#include <poly1305-donna.h>
#include <chacha20_simple.h>
#include <sodium.h>
extern void curve25519_donna(unsigned char *output, const unsigned char *a,
                             const unsigned char *b);



void print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { printf (" "); }
	}
	printf ("\n");

}

void 
hexprint ( unsigned char *str, uint32_t len ){
	int i;
	unsigned char p;
	printf ( "\n %4s: ", " " );		

	for (i=0; i<len; i++) {
		p = str[i];
		if ( p == 0 ) {
			printf ("_");
		} else if ( p < 32 ) {
			printf (" ");
		} else {
			printf ("%c", p);
		}
		
		if ( (i > 0 ) && (i % 42) == 0 ) {
			printf ( "\n %4s: ", " " );		
		}
	}

	printf ("\n");

}

int main(int argc, char**argv)
{

	int i;
	
	char *_lyrics = "You lit the fire  Then drank the water  You slammed that door and left me standing all alone  We wrote the story  We turned the pages  You changed the end like everybody said you would    I should have seen it coming  It should have sent me running  That's what I get for loving you    If I had a time machine and  If life was a movie scene  I'd rewind and I'd tell me run  We were never meant to be  So if I had a time machine  I'd go back and I'd tell me run run    - TIME MACHINE, WRITTEN BY INGRID MICHAELSON, BUSBEE, TRENT DABBS";
	int L = 533;	
	
	static const uint8_t basepoint[32] = {9};

	unsigned char mypublic[32];
	unsigned char mysecret[32];

	unsigned char theirpublic[32];
	unsigned char theirsecret[32];


	for (i=0; i<32; i++){
		mypublic[i] = 0;
		mysecret[i] =0;
		theirpublic[i]=0;
		theirsecret[i]=0;	
	}

	arc4random_buf(&mysecret, 32);
	arc4random_buf(&theirsecret, 32);
	uint8_t nonce[12]; 

	arc4random_buf(&nonce, 12);
	

	unsigned char shared[32];
	unsigned char shared2[32];


	curve25519_donna((unsigned char *)&mypublic, (unsigned char *)&mysecret, (unsigned char *)&basepoint);
	printf (" My Public Key : " );
	print_bin2hex((unsigned char *)&mypublic, 32);

	printf (" My Secret Key : " );
	print_bin2hex((unsigned char *)&mysecret, 32);


	curve25519_donna((unsigned char *)&theirpublic, (unsigned char *)&theirsecret, (unsigned char *)&basepoint);
	printf (" Their Public Key : " );
	print_bin2hex((unsigned char *)&theirpublic, 32);

	printf (" Their Secret Key : " );
	print_bin2hex((unsigned char *)&theirsecret, 32);
	
	curve25519_donna((unsigned char *)&shared, (unsigned char *)&mysecret, (unsigned char *)&theirpublic);
	printf (" Shared Key : " );
	print_bin2hex((unsigned char *)&shared, 32);

	curve25519_donna((unsigned char *)&shared2, (unsigned char *)&theirsecret, (unsigned char *)&mypublic );
	printf (" Shared Key 2: " );
	print_bin2hex((unsigned char *)&shared2, 32);

	
	printf (" Nonce : " ) ;
	print_bin2hex((unsigned char *)&nonce, 12);

	unsigned char mac[16];
	poly1305_auth(mac, (const unsigned char *)_lyrics, L, (const unsigned char *) &shared);

	printf("MAC : ");		
	print_bin2hex((unsigned char *)&mac, 16);
	
	unsigned char *data=calloc(L+16,sizeof(unsigned char));
	unsigned char *dataout=calloc(L+16,sizeof(unsigned char));
	memcpy(data, &mac, 16);
	memcpy(data+16, _lyrics, L);
	
	printf("Plain Text : %d \n", L);		
	hexprint( data, L+16);
	
	printf ( "\n\n");
	
	if (s20_crypt((uint8_t*)&shared, S20_KEYLEN_256, nonce, 0, data, L+16)) {
		printf("Error: encryption failed (salsa20) \n");
	} else {
		printf("Successful encryption (salsa20) \n");
		hexprint( data, L+16);
		printf(" \n");
	}

	unsigned char * data2=calloc(L+16,sizeof(unsigned char));
	unsigned char * data2out=calloc(L+16,sizeof(unsigned char));
	memcpy(data2, data, L+16);

	
	if (s20_crypt((uint8_t*)&shared, S20_KEYLEN_256, nonce, 0, data2, L+16)) {
		printf("Error: decryption failed\n");
	} else {
		printf("Successful decryption \n");
		hexprint( data2, L+16);		
		printf(" \n");
	}

	unsigned char * plaintext=calloc(L,sizeof(unsigned char));
	
	unsigned char mac2[16];
//	memcpy(&mac2, data, 16);
	memcpy(plaintext, data2+16, L);
//	plaintext[0] = 'D';
//	plaintext[1] = 'P';
//	plaintext[2] = 'D';
	
	printf("Plain Text : %d \n", L);		
	hexprint( plaintext, L);
	
	poly1305_auth(mac2, plaintext, L, (const unsigned char *)&shared);
	printf("MAC : ");		
	print_bin2hex((unsigned char *)&mac2, 16);	

	
	int r = poly1305_verify(mac2, mac);
	
	
	printf("MAC Verify (%d) (%s)\n", r, r ? "correct" : "incorrect");
	
	
	free(data2);
	free(data);


	printf ( " ===================> AEAD LibSodium <================================ \n " );

	unsigned char *ciphertext = calloc( L + crypto_aead_chacha20poly1305_ABYTES, sizeof(unsigned char) );
	unsigned long long ciphertext_len = L + crypto_aead_chacha20poly1305_ABYTES;

	unsigned char additional_data[3] = { 'D', 'P', 'D'};
	unsigned long long additional_data_length = 3;
	
	crypto_aead_chacha20poly1305_encrypt(ciphertext, &ciphertext_len,
										 (const unsigned char *)_lyrics, L,
										 (const unsigned char *)&additional_data, additional_data_length,
//										NULL, 0,
										 NULL, nonce, (unsigned char *) &shared);
									 
	printf ( " ===================> encrypted message <================================ \n" );
	hexprint( ciphertext, ciphertext_len);
	print_bin2hex( ciphertext, ciphertext_len);
	printf ( " Possible tag 1 :" );                                     
	print_bin2hex(ciphertext, 16);
	
	printf ( " Possible tag 2 :" );                                     
	print_bin2hex(ciphertext+ (ciphertext_len-16), 16);
	printf("\n");
	printf("\n");

	unsigned char *decrypted = calloc( L , sizeof(unsigned char) );
	unsigned long long decrypted_len = L ;
	unsigned char * additional_data2 = calloc(additional_data_length,sizeof(unsigned char));
//	unsigned long long additional_data_length2 =24;
		

	if (crypto_aead_chacha20poly1305_decrypt(decrypted, &decrypted_len,
											 NULL,
											 ciphertext, ciphertext_len,
											 (unsigned char *)&additional_data, additional_data_length,
//											 NULL, 0,
											 nonce, (unsigned char *)&shared) == 0) 
		{ 
		
			printf ( "Message good! \n" );
			printf("\n");
			printf ( "Original  Message[%d]:\n", L );
			hexprint( (unsigned char *)_lyrics, L);
			
			printf ( "Decrypted Message[%llu]:\n", decrypted_len );			
			hexprint( decrypted, decrypted_len);
	
		} else {
			printf ( "Message bad! \n" );
	
		}

//	free(data2);
//	free(data);
//	free(ciphertext);
//	free(decrypted);
//	free(additional_data2);

	printf ( " ===================> AEAD IETF LibSodium <================================ \n " );
	printf ( " ===================> AEAD IETF LibSodium <================================ \n " );
	printf ( " ===================> AEAD IETF LibSodium <================================ \n " );
	printf ( " ===================> AEAD IETF LibSodium <================================ \n " );
	printf ( " ===================> AEAD IETF LibSodium <================================ \n " );
	
	printf ( " ===================> AEAD IETF LibSodium <================================ \n " );

	ciphertext = calloc( L + crypto_aead_chacha20poly1305_ABYTES, sizeof(unsigned char) );
	ciphertext_len = L + crypto_aead_chacha20poly1305_ABYTES;

	//additional_data[3] = { 'D', 'P', 'D'};
	//unsigned long long additional_data_length = 3;
	
	crypto_aead_chacha20poly1305_ietf_encrypt(ciphertext, &ciphertext_len,
										 (const unsigned char *)_lyrics, L,
										 (const unsigned char *)&additional_data, additional_data_length,
//										NULL, 0,
										 NULL, nonce, (unsigned char *) &shared);
									 
	printf ( " ===================> encrypted message <================================ \n" );
	hexprint( ciphertext, ciphertext_len);
	print_bin2hex( ciphertext, ciphertext_len);
	printf ( " Possible tag 1 :" );                                     
	print_bin2hex(ciphertext, 16);
	
	printf ( " Possible tag 2 :" );                                     
	print_bin2hex(ciphertext+ (ciphertext_len-16), 16);
	printf("\n");
	printf("\n");





	decrypted = calloc( L , sizeof(unsigned char) );
	decrypted_len = L ;
	additional_data2 = calloc(additional_data_length,sizeof(unsigned char));
		




	if (crypto_aead_chacha20poly1305_ietf_decrypt(decrypted, &decrypted_len,
											 NULL,
											 ciphertext, ciphertext_len,
											 (unsigned char *)&additional_data, additional_data_length,
//											 NULL, 0,
											 nonce, (unsigned char *)&shared) == 0) 
		{ 
		
			printf ( "Message good! \n" );
			printf("\n");
			printf ( "Original  Message[%d]:\n", L );
			hexprint( (unsigned char *)_lyrics, L);
			
			printf ( "Decrypted Message[%llu]:\n", decrypted_len );			
			hexprint( decrypted, decrypted_len);
	
		} else {
			printf ( "Message bad! \n" );
	
		}




  
}