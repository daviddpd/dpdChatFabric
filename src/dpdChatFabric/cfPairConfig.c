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

*/

#include "cfPairConfig.h"

#ifdef ESP8266
#include "esp8266.h"
#endif
extern int _GLOBAL_DEBUG;

void CP_ICACHE_FLASH_ATTR
cfPairInit(chatFabricPairing *pair) 
{

	pair->hasPublicKey = 0;	
	pair->hasNonce = 0;
	pair->serial = 0;
	uuidCreateNil ( &(pair->uuid.u0) );
	uuidCreateNil ( &(pair->uuid.u1));

	pair->state = STATE_UNCONFIGURED;

	bzero(&(pair->nullnonce), crypto_secretbox_NONCEBYTES);
	bzero(&(pair->nonce), crypto_secretbox_NONCEBYTES);

#ifdef ESP8266
	arc4random_buf((unsigned char *)&(pair->mynonce), crypto_secretbox_NONCEBYTES);
#else
	arc4random_buf(&(pair->mynonce), crypto_secretbox_NONCEBYTES);
#endif

}


void CP_ICACHE_FLASH_ATTR
cfPairWrite(chatFabricConfig *config, chatFabricPairing *pair) 
{

	int len =0, i=0, filesize=0;
	uint32_t ni;
	struct stat fs;
	unsigned char *str;
	unsigned char t;	

    len+=1+crypto_box_PUBLICKEYBYTES;
    len+=1+crypto_secretbox_NONCEBYTES;
    len+=1+crypto_secretbox_NONCEBYTES;
    
    len+=1+16; // uuid
    len+=1+16;
    
    len+=1+1;  // flags - pk, nonce, state
    len+=1+1;
    len+=1+1;
    
    len+=1+4; // header
    len+=1+4; // pairs
    len+=1+4; // length
    
    len+=1+4; // serial

#ifdef ESP8266
	int fp=0;
	str = &(flashConfig[0]);
	fp = 1;
	i=2048;
#else
	FILE *fp=0;
	fp = fopen(config->pairfile,"w");
	if ( fp == 0 ) { 
// 		CHATFABRIC_DEBUG_FMT(config->debug,  
// 			"[DEBUG][%s:%s:%d] Cannot open pairfile for writing %s \n", 
// 			__FILE__, __FUNCTION__, __LINE__, config->pairfile );
		return;
	}
	str=(unsigned char *)calloc(len,sizeof(unsigned char));
	i=0;
#endif
				
	
	cfTagEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_header, 0, NULL, 0, NULL);			
	cfTagEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_pairs, 1, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_pairLength, len, NULL, 0, NULL);
// 	CHATFABRIC_DEBUG_FMT(config->debug,
// 		"[DEBUG][%s:%s:%d] Encoding Pair Config Length %d\n",
// 		__FILE__, __FUNCTION__, __LINE__, len );

	cfTagEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_hasPublicKey, 0, (unsigned char *)&pair->hasPublicKey, 1, NULL);
	cfTagEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_hasNonce, 0, (unsigned char *)&pair->hasNonce, 1, NULL);
	cfTagEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_state, 0, (unsigned char *)&pair->state, 1, NULL);

	cfTagEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_serial, pair->serial, NULL, 0, NULL);

	cfTagEncoder ( CP_UUID, str, (uint32_t *)&i, cftag_uuid0, 0, NULL, 0,  &pair->uuid.u0);
	cfTagEncoder ( CP_UUID, str, (uint32_t *)&i, cftag_uuid1, 0, NULL, 0,  &pair->uuid.u1);						

	cfTagEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_publickey, 0,(unsigned char *)&(pair->publickey), crypto_box_PUBLICKEYBYTES, NULL);
	cfTagEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_mynonce, 0, (unsigned char *)&(pair->mynonce), crypto_secretbox_NONCEBYTES, NULL);
	cfTagEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_nonce, 0, (unsigned char *)&(pair->nonce), crypto_secretbox_NONCEBYTES, NULL);

	CHATFABRIC_DEBUG_FMT(config->debug, "%20s: %s ",  "========>" , "cfPairWrite" );			
	CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG,"shared key", (unsigned char *)&pair->sharedkey, crypto_box_PUBLICKEYBYTES );
	CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG,"nonce", (unsigned char *)&pair->nonce, crypto_secretbox_NONCEBYTES);
	CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG,"mynonce", (unsigned char *)&pair->mynonce, crypto_secretbox_NONCEBYTES);


#ifdef ESP8266
	// FIXME :: should this be factored out into a writing module, so that multiple pairs 
	//			can be saved into the flashConfig block.
	if ( system_param_save_with_protect (CP_ESP_PARAM_START_SEC, &(flashConfig[0]), 4096) == FALSE ) {
// 		CHATFABRIC_DEBUG_FMT(config->debug,
// 			"[DEBUG][%s:%s:%d] Failed to Save Config to Flash\n",
// 			__FILE__, __FUNCTION__, __LINE__ );
		return;
	} else {
// 		CHATFABRIC_DEBUG_FMT(config->debug,  
// 			"[DEBUG][%s:%s:%d] Save Succesful, pair length: %d\n", 
// 			__FILE__, __FUNCTION__, __LINE__, len );		
	}

#else

	int fwi = fwrite (str, sizeof (unsigned char), len, fp );

// 	if ( fwi == 0 ) {
// 		CHATFABRIC_DEBUG_FMT(config->debug,  
// 			"[DEBUG][%s:%s:%d] Pair Config Write fw:(%d):len:(%d) \n",
// 			__FILE__, __FUNCTION__, __LINE__, fwi, len );
// 	}
	int fci = fclose(fp);

// 	if ( fci != 0 ) {	
// 		CHATFABRIC_DEBUG_FMT(config->debug,  
// 			"[DEBUG][%s:%s:%d] pair Config Write ERROR  errno %d, fci=%d %s  =%s=\n",
// 			__FILE__, __FUNCTION__, __LINE__,  errno, fci, strerror(errno), config->pairfile  );
// 	}
	
	free(str);
#endif
}


void CP_ICACHE_FLASH_ATTR
cfPairRead(chatFabricConfig *config, chatFabricPairing *pair) 
{

	struct stat fs;
	uint32_t ni;
	int len =0, i=0, filesize=0;
	unsigned char *str;
	unsigned char t;


#ifdef ESP8266
	if ( system_param_load (CP_ESP_PARAM_START_SEC, 0, &(flashConfig), 4096) == FALSE ) {
//		CHATFABRIC_DEBUG_FMT(config->debug, "Read from flash failed." ); 
		return;
	}

	if ( flashConfig[0] == cftag_header ) {
		filesize=4096;	
		//config->configfile = "1";
		str = &(flashConfig[0]);
		i=2048;

	} else {

// 		CHATFABRIC_DEBUG_FMT(config->debug,  
// 			"[DEBUG][%s:%s:%d] No Saved configuration in flash. \n", 
// 			__FILE__, __FUNCTION__, __LINE__ );
		return;

	}
#else
	FILE *fp=0;

	bzero(&fs, sizeof(fs));		
	fp = fopen(config->pairfile,"r");	
	if ( fp == NULL ) {
 		printf(
 			"[DEBUG][%s:%s:%d] Cannot open pairfile %s \n", 
 			__FILE__, __FUNCTION__, __LINE__, config->pairfile );
			return;
	} else {
		stat(config->pairfile, &fs);
		str=(unsigned char *)calloc(fs.st_size,sizeof(unsigned char));
		fread(str, sizeof (unsigned char), fs.st_size, fp );
		filesize=fs.st_size;
		fclose(fp);
	}			

#endif

	while (i<filesize) 
	{
		memcpy(&t, str+i, 1);
		++i;			
		switch (t){
			case cftag_header:
				i+=4;
			break;					
			case cftag_pairLength:
				memcpy(&ni, str+i, 4);
				i+=4;
#ifndef ESP8266
				filesize = ntohl(ni);
#else
				filesize = 2048 + ntohl(ni);
#endif
				
			printf(   
 				"[DEBUG][%s:%s:%d] Got Length as  %02x %08x %4d\n", 
 				__FILE__, __FUNCTION__, __LINE__, t, ni, filesize);
			break;
			case cftag_pairs:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->hasPairs = 1;
				// FIXME: Feature add, multiple pair support
			break;
			case cftag_publickey:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(pair->publickey), str+i, crypto_box_PUBLICKEYBYTES);
				i+=crypto_box_PUBLICKEYBYTES;
                pair->state = STATE_PAIRED;
//				curve25519_donna((unsigned char *)&pair->sharedkey, (unsigned char *)&config->privatekey, (unsigned char *)&pair->publickey);                
			break;		
			case cftag_nonce:		// 1+crypto_secretbox_NONCEBYTES
				memcpy(&(pair->nonce), str+i, crypto_secretbox_NONCEBYTES);
				i += crypto_secretbox_NONCEBYTES;					
			break;		
			case cftag_mynonce:		// 1+crypto_secretbox_NONCEBYTES
				memcpy(&(pair->mynonce), str+i, crypto_secretbox_NONCEBYTES);
				i += crypto_secretbox_NONCEBYTES;					
			break;
			case cftag_serial:
				memcpy(&ni, str+i, 4);
				i+=4;
				pair->serial = ntohl(ni);
			break;					
			case cftag_uuid0:			// 1+16
				uuidFromBytes(str+i, &pair->uuid.u0);
				i+=16;
			break;		
			case cftag_uuid1:			// 1+16
				uuidFromBytes(str+i, &pair->uuid.u1);
				i+=16;
			break;		

			case cftag_hasPublicKey:	 // 1+1
				memcpy(&(pair->hasPublicKey), str+i, 1);
				++i;
			break;		
			case cftag_hasNonce:		// 1+1
				memcpy(&(pair->hasNonce), str+i, 1);
				++i;
			break;		
			case cftag_state:		// 1+1
				memcpy(&(pair->state), str+i, 1);
				++i;
			break;
			default:
// 				CHATFABRIC_DEBUG_FMT(config->debug,  
// 					"[DEBUG][%s:%s:%d] Bad Pairing Config file Tag: %02x \n", 
// 					__FILE__, __FUNCTION__, __LINE__,  t );
			break;

		}
	}
					
	curve25519_donna((unsigned char *)&pair->sharedkey, (unsigned char *)&config->privatekey, (unsigned char *)&pair->publickey);

	CHATFABRIC_DEBUG_FMT(config->debug, "%20s: %s ",  "========>" , "cfPairRead" );			
	CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG,"shared key", (unsigned char *)&pair->sharedkey, crypto_box_PUBLICKEYBYTES );
	CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG,"nonce", (unsigned char *)&pair->nonce, crypto_secretbox_NONCEBYTES);
	CHATFABRIC_DEBUG_B2H(_GLOBAL_DEBUG,"mynonce", (unsigned char *)&pair->mynonce, crypto_secretbox_NONCEBYTES);
	


}

