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

#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "util.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ESP8266
#include "esp8266.h"
//#define __error (void)0
#endif



void CP_ICACHE_FLASH_ATTR
chatFabric_configParse(chatFabricConfig *config) 
{

#ifdef ESP8266
	int fp=0;
#else
	FILE *fp=0;
#endif
	struct stat fs;
	uint32_t ni;
	int len =0, i=0, filesize=0;
	unsigned char *str;
	unsigned char t;
//	enum chatFabricConfigTags t;
#ifdef ESP8266

	if ( system_param_load (CP_ESP_PARAM_START_SEC, 0, &(flashConfig), 4096) == FALSE ) {
		CHATFABRIC_DEBUG_FMT(config->debug, "Read from flash failed." ); 	
	}

	if ( flashConfig[0] == cftag_header ) {
		filesize=4096;	
		config->configfile = "1";
	} else {

		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] Unconfigured. \n", 
			__FILE__, __FUNCTION__, __LINE__ );

		uint32_t status;
		static const unsigned char basepoint[32] = {9};
        config->writeconfig = 1;
		config->hasPairs = 0;
        config->configfile = NULL;
		config->pairfile = NULL;
		config->callback = NULL;
		
		filesize=0;
		// These are created in args, so for the embedded solution
		// putting here.
		uuidCreateNil( &(config->to.u0));
		uuidCreateNil( &(config->to.u1));
		uuidCreateNil( &(config->uuid.u0));
		uuidCreate( &(config->uuid.u1));
		arc4random_buf((unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES);
		curve25519_donna((unsigned char *)&config->publickey, (unsigned char *)&config->privatekey, (unsigned char *)&basepoint);	

	}
	str = &(flashConfig[0]);
	
#endif

	
	if ( config->configfile != NULL ) 
	{
#ifndef ESP8266
			bzero(&fs, sizeof(fs));		
			fp = fopen(config->configfile,"r");	
			if ( fp == NULL ) {
				fprintf(stderr, " Error, can't open file %s \n", config->configfile );
				filesize=0;
			} else {
				stat(config->configfile, &fs);
				str=(unsigned char *)calloc(fs.st_size,sizeof(unsigned char));
				fread(str, sizeof (unsigned char), fs.st_size, fp );
				filesize=fs.st_size;
				fclose(fp);
			}			
#endif
			
			i=0;
			
			while (i<filesize) 
			{
				memcpy(&t, str+i, 1);
				++i;
							
				switch (t){
					case cftag_header:
						i+=4;
					break;
					
					case cftag_hasPairs:
						memcpy(&ni, str+i, 4);
						i+=4;
						config->hasPairs = ntohl(ni);
					break;

					case cftag_mode:
						memcpy(&ni, str+i, 4);
						i+=4;
						config->mode = ntohl(ni);
					break;
					case cftag_ipv4:
						memcpy(&ni, str+i, 4);
						i+=4;
						config->ipv4 = ntohl(ni);
					break;
					case cftag_ipv4netmask:
						memcpy(&ni, str+i, 4);
						i+=4;
						config->ipv4netmask = ntohl(ni);
					break;
					case cftag_ipv4gw:
						memcpy(&ni, str+i, 4);
						i+=4;
						config->ipv4gw = ntohl(ni);
					break;
					case cftag_ipv4ns1:
						memcpy(&ni, str+i, 4);
						i+=4;
						config->ipv4ns1 = ntohl(ni);
					break;
					case cftag_ipv4ns2:
						memcpy(&ni, str+i, 4);
						i+=4;
						config->ipv4ns2 = ntohl(ni);
					break;
								
					case cftag_configLength:
						memcpy(&ni, str+i, 4);
						i+=4;
						filesize = ntohl(ni);
					break;
					case cftag_publickey:	// 1+crypto_box_SECRETKEYBYTES
						memcpy(&(config->publickey), str+i, crypto_box_PUBLICKEYBYTES);
						i+=crypto_box_PUBLICKEYBYTES;
					break;		
					case cftag_privatekey:	// 1+crypto_box_SECRETKEYBYTES
						memcpy(&(config->privatekey), str+i, crypto_box_SECRETKEYBYTES);
						i+=crypto_box_SECRETKEYBYTES;
					break;		
					case cftag_uuid0:			// 1+16
						uuidFromBytes(str+i, &config->uuid.u0);
						i+=16;
					break;		
					case cftag_uuid1:			// 1+16
						uuidFromBytes(str+i, &config->uuid.u1);
						i+=16;
					break;		
					default:
						CHATFABRIC_DEBUG_FMT(config->debug,  
							"[DEBUG][%s:%s:%d] Bad Config File Tag : %02x \n", 
							__FILE__, __FUNCTION__, __LINE__,  t );
					break;
				}
			}
		}

	if ( config->writeconfig ) 
	{
		len+=1+crypto_box_PUBLICKEYBYTES;
		len+=1+crypto_box_SECRETKEYBYTES;
		len+=1+16; // uuid
		len+=1+16;
		len+=1+4; // header
		len+=1+4; // length
		len+=1+4; // haspairs
		
		len+=1+4; // mode
		len+=1+4; // ipv4
		len+=1+4; // ipv4nm
		len+=1+4; // ipv4gw
		len+=1+4; // ipv4ns1
		len+=1+4; // ipv4ns2
		
		
		

#ifdef ESP8266
		if ( 1 ) 
		{
			i=0; // FIXME: need to make this the correct offset in flash.

#else
		fp = fopen(config->newconfigfile,"w");		
		if ( fp != 0 )
		{
			i=0;
			str=(unsigned char *)calloc(len,sizeof(unsigned char));
#endif 	

			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_header, 0, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_configLength, len, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_hasPairs, config->hasPairs, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_UUID, str, (uint32_t *)&i, cftag_uuid0, 0, NULL, 0,  &config->uuid.u0);
			chatPacket_tagDataEncoder ( CP_UUID, str, (uint32_t *)&i, cftag_uuid1, 0, NULL, 0,  &config->uuid.u1);
			
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_mode, config->mode, NULL, 0, NULL);

			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_ipv4, config->ipv4, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_ipv4netmask, config->ipv4netmask, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_ipv4gw, config->ipv4gw, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_ipv4ns1, config->ipv4ns1, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_ipv4ns2, config->ipv4ns2, NULL, 0, NULL);
			
			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_publickey, 0,(unsigned char *)&(config->publickey), crypto_box_PUBLICKEYBYTES, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_privatekey, 0,(unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES, NULL);

#ifdef ESP8266
		if ( system_param_save_with_protect (CP_ESP_PARAM_START_SEC, &(flashConfig[0]), 4096) == FALSE ) {
			CHATFABRIC_DEBUG_FMT(1,  
				"[DEBUG][%s:%s:%d] Failed to Save Config to Flash\n", 
				__FILE__, __FUNCTION__, __LINE__ );
		} else {
			CHATFABRIC_DEBUG_FMT(1,  
				"[DEBUG][%s:%s:%d] Save Succesful.\n", 
				__FILE__, __FUNCTION__, __LINE__ );		
//			util_print_bin2hex((unsigned char *)&flashConfig, 256);	
				
		}

#else
			int fwi = fwrite (str, sizeof (unsigned char), len, fp );
			int fci = fclose(fp);
			CHATFABRIC_DEBUG_FMT(config->debug,  
				"[DEBUG][%s:%s:%d] cf Config Write (%d):(%d) \n",
				__FILE__, __FUNCTION__, __LINE__, fwi, fci );
			
			free(str);
#endif
		} else {
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] cf Config Write ERROR  errno %d, %s, =%s=\n",
			__FILE__, __FUNCTION__, __LINE__,  errno, strerror(errno), config->configfile  );
		
		}
	}

}

void CP_ICACHE_FLASH_ATTR
chatFabric_pair_init(chatFabricPairing *pair) 
{

	pair->state = STATE_UNCONFIGURED;
	pair->hasPublicKey = 0;	
	uuidCreateNil ( &(pair->uuid.u0) );
	uuidCreateNil ( &(pair->uuid.u1));
	pair->hasPublicKey = 0;
	pair->hasNonce = 0;
#ifdef ESP8266	
	arc4random_buf((unsigned char *)&(pair[0].mynonce), crypto_secretbox_NONCEBYTES);
#else
	arc4random_buf(&(pair->mynonce), crypto_secretbox_NONCEBYTES);
#endif

	bzero(&(pair->nullnonce), crypto_secretbox_NONCEBYTES);

}


void CP_ICACHE_FLASH_ATTR
chatFabric_pairConfig(chatFabricConfig *config, chatFabricPairing *pair, int write ) 
{
			CHATFABRIC_DEBUG(config->debug, "In pair config." );

#ifdef ESP8266
	int fp=0;
#else
	FILE *fp=0;
#endif
	int len =0, i=0, filesize=0;
	uint32_t ni;
	struct stat fs;
	unsigned char *str;
	unsigned char t;
//	enum chatFabricConfigTags t;
	
		
	if ( ( config->pairfile != NULL ) && (write == 1) )
	{ 		
		CHATFABRIC_DEBUG(config->debug, "going to write pairconf" );
#ifndef ESP8266
		fp = fopen(config->pairfile,"w");		
		str=(unsigned char *)calloc(len,sizeof(unsigned char));
		i=0;
#else 
//		CHATFABRIC_DEBUG(config->debug, "pointer assignment" );
		str = &(flashConfig[0]);
//		CHATFABRIC_DEBUG(config->debug, "setting fp to 1" );
		fp = 1;
//		CHATFABRIC_DEBUG(config->debug, "setting i to 2048" );
		i=2048;
#endif

		CHATFABRIC_DEBUG(config->debug, "if" );
		if ( fp != 0 )
		{
		
			CHATFABRIC_DEBUG(config->debug, "creating buffer" );
			
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
			
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_header, 0, NULL, 0, NULL);			
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_pairs, 1, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_pairLength, len, NULL, 0, NULL);
			CHATFABRIC_DEBUG_FMT(config->debug,
				"[DEBUG][%s:%s:%d] Encoding Pair Config Length %d\n",
				__FILE__, __FUNCTION__, __LINE__, len );

			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_hasPublicKey, 0, (unsigned char *)&pair->hasPublicKey, 1, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_hasNonce, 0, (unsigned char *)&pair->hasNonce, 1, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_state, 0, (unsigned char *)&pair->state, 1, NULL);

			chatPacket_tagDataEncoder ( CP_INT32, str, (uint32_t *)&i, cftag_serial, pair->serial, NULL, 0, NULL);

			chatPacket_tagDataEncoder ( CP_UUID, str, (uint32_t *)&i, cftag_uuid0, 0, NULL, 0,  &pair->uuid.u0);
			chatPacket_tagDataEncoder ( CP_UUID, str, (uint32_t *)&i, cftag_uuid1, 0, NULL, 0,  &pair->uuid.u1);						

			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_publickey, 0,(unsigned char *)&(pair->publickey), crypto_box_PUBLICKEYBYTES, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_mynonce, 0, (unsigned char *)&(pair->mynonce), crypto_secretbox_NONCEBYTES, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, (uint32_t *)&i, cftag_nonce, 0, (unsigned char *)&(pair->nonce), crypto_secretbox_NONCEBYTES, NULL);
#ifdef ESP8266
		if ( system_param_save_with_protect (CP_ESP_PARAM_START_SEC, &(flashConfig[0]), 4096) == FALSE ) {
			CHATFABRIC_DEBUG_FMT(config->debug,
				"[DEBUG][%s:%s:%d] Failed to Save Config to Flash\n",
				__FILE__, __FUNCTION__, __LINE__ );
		} else {
			CHATFABRIC_DEBUG_FMT(config->debug,  
				"[DEBUG][%s:%s:%d] Save Succesful.\n", 
				__FILE__, __FUNCTION__, __LINE__ );		
		}

#else
			int fwi = fwrite (str, sizeof (unsigned char), len, fp );
			int fci = fclose(fp);
			CHATFABRIC_DEBUG_FMT(config->debug,  
				"[DEBUG][%s:%s:%d] Pair Config Write (%d):(%d) \n",
				__FILE__, __FUNCTION__, __LINE__, fwi, fci );
			
			
#endif
		} else {
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] pair Config Write ERROR  errno %d, %s  =%s=\n",
			__FILE__, __FUNCTION__, __LINE__,  errno, strerror(errno), config->pairfile  );
		
			CHATFABRIC_DEBUG(config->debug, "fp null" );

		}
#ifndef ESP8266
		free(str);
#endif
	} else if ( ( config->pairfile != NULL ) && (write == 0) ) {

#ifndef ESP8266		
		fp = fopen(config->pairfile,"r");
#else
		fp = 1;
#endif
		if ( fp != 0 )
		{
#ifndef ESP8266		
		
			bzero(&fs, sizeof(fs));
			stat(config->pairfile, &fs);
			
			str=(unsigned char *)calloc(fs.st_size,sizeof(unsigned char));
			fread(str, sizeof (unsigned char), fs.st_size, fp );
			filesize=fs.st_size;

			i=0;
#else
			// FIXME:  Add if reconfig condition ... from button/GPIO ?
			if ( system_param_load (CP_ESP_PARAM_START_SEC, 0, &(flashConfig[0]), 4096) == TRUE ) {
				filesize=4096;	
				config->pairfile = "1";
			}
			str = &(flashConfig[0]);
			i=2048;
#endif
			
			while (i<filesize) 
			{
				memcpy(&t, str+i, 1);
/*				
				CHATFABRIC_DEBUG_FMT(config->debug,
					"[DEBUG][%s:%s:%d] Parsing File %02x %4d\n",
					__FILE__, __FUNCTION__, __LINE__, t, i);
*/					
				
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
						
					CHATFABRIC_DEBUG_FMT(config->debug,  
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
						CHATFABRIC_DEBUG_FMT(config->debug,  
							"[DEBUG][%s:%s:%d] Bad Pairing Config file Tag: %02x \n", 
							__FILE__, __FUNCTION__, __LINE__,  t );
					break;
		
				}
			}
			curve25519_donna((unsigned char *)&pair->sharedkey, (unsigned char *)&config->privatekey, (unsigned char *)&pair->publickey);

		} else {
			CHATFABRIC_DEBUG(config->debug, "Error Opening Pairing file." );		
		}
	
	}
	CHATFABRIC_DEBUG(config->debug, "return" );

}


#ifndef ESP8266

void CP_ICACHE_FLASH_ATTR
chatFabric_consetup( chatFabricConnection *c,  char *ip, int port )
{

	int lowwater = 64;
	
	if ( c->type == SOCK_STREAM  &&  c->acceptedSocket == -1 && c->socket != -1 && c->bind == 1) {
		socklen_t len;
		c->acceptedSocket = accept(c->socket,(struct sockaddr *)&c->sockaddr,&len);
		return;
	} else if ( c->type == SOCK_STREAM  &&  c->socket != -1 && c->bind == 0) {
		return;	
	} else if ( c->type == SOCK_DGRAM  &&  c->socket != -1 ) {
		return;	
	}
	// c->bind = doBind;
	c->acceptedSocket = -1;
	c->socket = -1;		
	c->socket=socket(AF_INET,c->type,0);

	CHATFABRIC_DEBUG_FMT(1,  
		"[DEBUG][%s:%s:%d] ERRNO: %d  Socket FD %d \n", 
		__FILE__, __FUNCTION__, __LINE__,  errno, c->socket );

	
	if (c->socket == -1 ) {
		return;
	}
	
	bzero( &(c->sockaddr),sizeof(c->sockaddr) );
	c->sockaddr.sin_family = AF_INET;
	if ( ip == 0 ) {
		c->sockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	} else {
		c->sockaddr.sin_addr.s_addr=inet_addr(ip);
	}
	c->sockaddr.sin_port=htons(port);
	
	if ( c->type == SOCK_DGRAM ) {
		setsockopt(c->socket, SOL_SOCKET, SO_RCVLOWAT, &lowwater, sizeof(lowwater));
	}

	if ( c->bind ==  1 ) {
		bind(c->socket,(struct sockaddr *)&c->sockaddr,sizeof(c->sockaddr));
		if ( c->type == SOCK_STREAM ) 
		{
			listen(c->socket,1024);
			socklen_t len;
			c->acceptedSocket = accept(c->socket,(struct sockaddr *)&c->sockaddr,&len);
		}
	} else {
		if ( c->type == SOCK_STREAM ) {
			if ( connect(c->socket, (struct sockaddr *)&c->sockaddr, sizeof(c->sockaddr) ) < 0 ) {
			CHATFABRIC_DEBUG_FMT(1,  
				"[DEBUG][%s:%s:%d] ERRNO: %d  Socket Connect FD %d , %s \n", 
				__FILE__, __FUNCTION__, __LINE__,  errno, c->socket, strerror(errno) );
//				assert(0);
			
			}

		}
	}
	
	return;
} 

#endif


enum chatFabricErrors CP_ICACHE_FLASH_ATTR
chatFabric_controller(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config, chatFabricAction *a , msgbuffer *b) 
{
	int n;
	socklen_t len;
	chatPacket *cp;
	msgbuffer mb;
	unsigned char * nullmsg = 0;
	b->length = 0;		
	enum chatFabricErrors e;
#ifndef ESP8266
	chatFabric_consetup(c, config->ip, config->port);
	if ( c->socket == -1 ) {
		CHATFABRIC_DEBUG(config->debug, "chatFabric connection setup failed" );
		return ERROR_SOCKET;
	}	
#endif

	if ( pair->state != STATE_PAIRED ) {
		cp = chatPacket_init (config, pair, CMD_PAIR_REQUEST,  nullmsg, 0,  0);
		chatPacket_encode ( cp, config, pair,  &mb, _CHATPACKET_ENCRYPTED, COMMAND);
	} else {
		if ( a->action == ACTION_APP_LIST ) {
			cp = chatPacket_init (config, pair, CMD_APP_LIST,  NULL, 0,  CMD_SEND_REPLY_TRUE);
			
		} else if ( config->msg == 0 || config->msg == NULL ) {
			cp = chatPacket_init (config, pair, CMD_APP_MESSAGE,  NULL, 0,  CMD_SEND_REPLY_TRUE);
		} else { 
			cp = chatPacket_init (config, pair, CMD_APP_MESSAGE,  config->msg, strlen((const char *)config->msg),  CMD_SEND_REPLY_TRUE);
		}
		cp->action = a->action;
		cp->action_control = a->action_control;
		cp->action_type = a->action_type;
		cp->action_value = a->action_value;
		cp->action_length = a->action_length;

		chatPacket_encode ( cp, config, pair,  &mb, _CHATPACKET_ENCRYPTED, DATA);
	}
#ifdef ESP8266
		espconn_sent(c->conn, (uint8 *)mb.msg, mb.length);
#else
	len = sizeof(c->sockaddr);
	
	if ( c->type == SOCK_STREAM ) {
		if (c->bind == 1 ) {
		    n = write(c->acceptedSocket, mb.msg, mb.length);
		} else {
		    n = write(c->socket, mb.msg, mb.length);		
		}
	} else {	
		n = sendto(c->socket, mb.msg, mb.length, 0, (struct sockaddr *)&(c->sockaddr), len);
	}
#endif

	if ( config->debug )
		chatPacket_print(cp, OUT);
						
//#ifndef ESP8266
	free(mb.msg);
//#endif
	mb.length = 0;
	chatPacket_delete(cp);
	
	int getAppList = 0;
	
	if (pair->state != STATE_PAIRED) {
		getAppList = 1;	
	}
	
	do { 
		e = chatFabric_device(c, pair, config, b);
	} while ( (e == ERROR_OK) && (pair->state != STATE_PAIRED)  );
	
/*	
	if ( getAppList ) {
		a->action = ACTION_APP_LIST;
		chatFabric_controller(c, pair, config, a, b);

	}
*/	
#ifndef ESP8266	
	//close (c->socket);
#endif
	return e;
	
}

enum chatFabricErrors CP_ICACHE_FLASH_ATTR
chatFabric_device(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config, msgbuffer *b) 
{
	CHATFABRIC_DEBUG(config->debug, " Start " );

	int n, buffersize=1460;
	unsigned char *mesg;
	socklen_t len;
	chatPacket *cp, *cp_reply;
	msgbuffer mb;
	b->length = 0;	

	enum chatPacketCommands replyCmd;
	enum chatPacketPacketTypes cptype;
	enum chatFabricErrors e;
	
#ifdef ESP8266
	n = mbuff.length;
	mesg = mbuff.msg;
	int errno = 0;
#else
	if ( c->socket == -1 || ( c->acceptedSocket == -1 && c->bind == 1 )  ) {
		chatFabric_consetup(c, config->ip, config->port);
		if ( c->socket == -1 ) {
			CHATFABRIC_DEBUG(config->debug, "chatFabric connection setup failed" );
			return ERROR_SOCKET;
		}	
	}

	mesg=(unsigned char *)calloc(buffersize,sizeof(unsigned char));

	CHATFABRIC_DEBUG(config->debug, "Waiting for Packet.\n\n" );

	len = sizeof(c->sockaddr);

	if ( c->type == SOCK_STREAM ) { 
		if ( c->bind == 1 ) {
			n = read(c->acceptedSocket,mesg,buffersize);
		} else {
			n = read(c->socket,mesg,buffersize);
		}
		// n = recvfrom(c->acceptedSocket,mesg,buffersize,0,(struct sockaddr *)&(c->sockaddr),&len);
	} else {
		n = recvfrom(c->socket,mesg,buffersize,0,(struct sockaddr *)&(c->sockaddr),&len);
	}
#endif 	
	if ( n == -1 ) {
#ifndef ESP8266
		free(mesg); /* needed */ 
#endif
		CHATFABRIC_DEBUG(config->debug, "recvfrom returned an error" );
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] ERRNO: %d \n", 
			__FILE__, __FUNCTION__, __LINE__,  errno );
//		assert (0);
		return ERROR_OK;	
	} else {
		CHATFABRIC_DEBUG(config->debug, "Got Packet!" );	
	}

	cp = chatPacket_init0 ();

	if ( chatPacket_decode (cp, pair, mesg, n, config ) != 0 ) {
		CHATFABRIC_DEBUG(config->debug, "chatPacket decoding failed." );
#ifndef ESP8266
		free(mesg);
#endif
		chatPacket_delete(cp);
		return ERROR_CHATPACKET_DECODING;		
	} else {
	

#ifndef ESP8266
		if ( config->debug )
			chatPacket_print (cp, IN);
#endif

		if ( cp->payloadLength > 0 )
		{
			b->length = cp->payloadLength;
			b->msg=(unsigned char*)calloc(b->length,sizeof(unsigned char));		
			memcpy(b->msg, cp->payload, b->length);
		} else {
			b->length = 0;
		}

		if ( cp->action != ACTION_NULL ) {
			b->action = cp->action;
			b->action_control = cp->action_control;
			b->action_type = cp->action_type;
			b->action_value = cp->action_value;
			b->action_length = cp->action_length;
		}

#ifndef ESP8266		
		free(mesg);
#endif
	}
	
	CHATFABRIC_DEBUG(config->debug, " == cp init0 " );
	cp_reply = chatPacket_init0 ();
	
	CHATFABRIC_DEBUG(config->debug, " == starting state machine == \n " );
	stateMachine ( config, cp, pair, cp_reply, &replyCmd, &e);
	if ( config->callback != NULL && pair->state == STATE_PAIRED && 
		( 
			cp->cmd == CMD_APP_MESSAGE || cp->cmd == CMD_APP_MESSAGE_ACK || 
			cp->cmd == CMD_APP_LIST || cp->cmd == CMD_APP_LIST_ACK 
		) 
	) {
		config->callback(config, cp, pair, cp_reply, &replyCmd);	
	}
	if ( replyCmd == CMD_SEND_REPLY_TRUE ) {

#ifndef ESP8266
		if ( config->debug )
			chatPacket_print (cp_reply, OUT);
#endif

		switch (cp_reply->cmd) {
			case CMD_NONCE_SEND:
				cptype = NONCE;
			break;
			case CMD_PUBLICKEY_SEND:
				cptype = PUBLICKEY;
			break;
			case CMD_APP_MESSAGE:
			case CMD_APP_MESSAGE_ACK:
			case CMD_APP_REGISTER:
			case CMD_APP_LIST:
			case CMD_APP_LIST_ACK:
				cptype = DATA;
			break;			
			default:
				cptype = COMMAND;
			break;
		}
		chatPacket_encode ( cp_reply, config, pair, &mb, _CHATPACKET_ENCRYPTED, cptype);
		// FIXME: Error check and handle sending error
#ifdef ESP8266
		espconn_sent(c->conn, (uint8 *)mb.msg, mb.length);
#else
	if ( c->type == SOCK_STREAM ) {
		if ( c->bind == 1 ) 
		{
		    n = write(c->acceptedSocket, mb.msg, mb.length);
		} else {
		    n = write(c->socket, mb.msg, mb.length);		
		}
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] chatPacket TCP data sent. Bytes: %d  errno %d, socket %d %d  SOCK_TYPE : %d  %s \n", 
			__FILE__, __FUNCTION__, __LINE__,  n, errno,c->acceptedSocket, c->socket, c->type, strerror(errno)  );
	} else {
	
		n = sendto(c->socket, mb.msg, mb.length, 0, (struct sockaddr *)&(c->sockaddr), len);	
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] chatPacket UDP data sent. Bytes: %d  errno %d, socket %d %d  SOCK_TYPE : %d  %s \n", 
			__FILE__, __FUNCTION__, __LINE__,  n, errno,c->acceptedSocket, c->socket, c->type, strerror(errno)  );
	}
#endif

//#ifndef ESP8266
		free(mb.msg);
//#endif
		mb.length = 0;
		chatPacket_delete(cp_reply);
	} else {
		chatPacket_delete(cp_reply);	
	}
	
#ifndef ESP8266
	if ( ( c->type == SOCK_STREAM ) &&  (pair->state == STATE_PAIRED)  )
	{
		if ( c->acceptedSocket != -1 ) {
			close(c->acceptedSocket);
			c->acceptedSocket = -1;
		} else if ( c->socket != -1 ) {
		
//			close(c->socket);
//			c->socket = -1;
	
		}
		
		
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] chatPacket TCP CLOSING Accepted Socket.  socket %d %d  SOCK_TYPE : %d  %s \n", 
			__FILE__, __FUNCTION__, __LINE__,  c->acceptedSocket, c->socket, c->type, strerror(errno)  );
	}
#endif	
	chatPacket_delete(cp);
	CHATFABRIC_DEBUG(config->debug, "function return." );	
	return e;
}



void CP_ICACHE_FLASH_ATTR
stateMachine (chatFabricConfig *config, chatPacket *cp, chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd, enum chatFabricErrors *e)
{	
	CHATFABRIC_DEBUG(config->debug, " Start " );

	chatFabricPairing  previous_state;
	
	enum chatPacketCommands RETVAL;
	int u0 = uuidCompare(&(cp->to.u0), &(config->uuid.u0));
	int u1 = uuidCompare(&(cp->to.u1), &(config->uuid.u1));
	CHATFABRIC_DEBUG_FMT(config->debug,  
		"[DEBUG][%s:%s:%d] u0 %d u1 %d \n", 
		__FILE__, __FUNCTION__, __LINE__,  u0, u1 );
	
	if (  
		( ( u0 !=0 ) || ( u1 != 0 )  ) &&
		( cp->cmd != CMD_HELLO) && 
		( cp->cmd != CMD_HELLO_ACK) && 
		( cp->cmd != CMD_PAIR_REQUEST) 
	 ) 
	{
		CHATFABRIC_DEBUG(config->debug, " if " );
		*e = ERROR_INVAILD_DEST;
		RETVAL = CMD_SEND_REPLY_FALSE;
		return;
	} else {
		CHATFABRIC_DEBUG(config->debug, " else " );
	}


	previous_state.state =  pair->state;
	previous_state.hasPublicKey = pair->hasPublicKey;

	uuidCopy( &cp->from.u0, &reply->to.u0);
	uuidCopy( &cp->from.u1, &reply->to.u1);
	uuidCopy( &config->uuid.u0, &reply->from.u0);
	uuidCopy( &config->uuid.u1, &reply->from.u1);
	
	RETVAL = CMD_SEND_REPLY_FALSE;
	
	CHATFABRIC_DEBUG_FMT(config->debug,  
		"[DEBUG][%s:%s:%d] STATE: %s COMMAND: %s \n", 
		__FILE__, __FUNCTION__, __LINE__,  cmdLookup(cp->cmd), stateLookup(pair->state) );


	switch ( cp->cmd ) {

		case CMD_PAIR_REQUEST_RESET:
			if ( pair->state != STATE_PAIRED ) {
				pair->state = STATE_UNCONFIGURED;
				reply->flags = pair->state;
				reply->cmd = CMD_PAIR_REQUEST;
				RETVAL = CMD_SEND_REPLY_TRUE;
			}
		break;
		case CMD_HELLO:
			reply->flags = pair->state;
			reply->cmd = CMD_HELLO_ACK;
			RETVAL = CMD_SEND_REPLY_TRUE;
		break;	

		case CMD_HELLO_ACK:
			reply->flags =  pair->state;
			reply->cmd = CMD_HELLO_ACK;
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;	
		case CMD_PAIR_REQUEST:
			if ( pair->state == STATE_UNCONFIGURED ) {
				reply->flags = 0;
				reply->cmd = CMD_PAIR_REQUEST_ACK;
				pair->state = STATE_PUBLICKEY_SETUP;
				uuidCopy( &cp->from.u0, &pair->uuid.u0);
				uuidCopy( &cp->from.u1, &pair->uuid.u1);
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
		case CMD_PAIR_REQUEST_ACK:
			if ( pair->state == STATE_UNCONFIGURED ) {
				reply->flags = 0;
				reply->cmd = CMD_PUBLICKEY_REQUEST;
				pair->state = STATE_PUBLICKEY_SETUP;
				uuidCopy( &cp->from.u0, &pair->uuid.u0);
				uuidCopy( &cp->from.u1, &pair->uuid.u1);
				RETVAL = CMD_SEND_REPLY_TRUE;
			}
		break;
		case CMD_NONCE_REQUEST:
			if  ( pair->state == STATE_NONCE_SETUP ) {
				if ( pair->hasPublicKey == 1 ) {
					reply->flags = pair->hasNonce;
//					reply->nonce = pair->mynonce;
					memcpy(&(reply->nonce), &(pair->mynonce), crypto_secretbox_NONCEBYTES );
					reply->cmd = CMD_NONCE_SEND;
					pair->state = STATE_NONCE_SETUP;
					RETVAL = CMD_SEND_REPLY_TRUE;
				} else {
					reply->flags = 0;
					pair->state = STATE_UNCONFIGURED;
					reply->cmd = CMD_PAIR_REQUEST_RESET;
					RETVAL = CMD_SEND_REPLY_TRUE;
				}
			}
		break;
		case CMD_NONCE_ACK:
			if  ( pair->state == STATE_NONCE_SETUP ) {
					pair->state = STATE_PAIRED;
//					reply->cmd = CMD_APP_LIST;
					RETVAL = CMD_SEND_REPLY_FALSE;
//					a->action = (uint32_t )ACTION_GET;
//					a->action_control = 0;
//					a->action_value = 0;
			}		
		break;
		case CMD_NONCE_SEND:
			if  ( pair->state == STATE_NONCE_SETUP ) {
//				pair->nonce = cp->nonce;
				memcpy( &(pair->nonce), &(cp->nonce), crypto_secretbox_NONCEBYTES );
				
				pair->hasNonce = 1;
				reply->flags = 0;
				if ( cp->flags == 1 ) {
					reply->cmd = CMD_NONCE_ACK;
					pair->state = STATE_PAIRED;				
				} else {
					reply->flags = pair->hasNonce;
					reply->cmd = CMD_NONCE_SEND;
//					reply->nonce = pair->mynonce;
					memcpy(&(reply->nonce), &(pair->mynonce), crypto_secretbox_NONCEBYTES );
					pair->state = STATE_NONCE_SETUP;
				}
				RETVAL = CMD_SEND_REPLY_TRUE;				
			}			
		break;
		case CMD_NONCE_RESET:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_PUBLICKEY_ACK:
			if  ( pair->state == STATE_PUBLICKEY_SETUP ) {
				pair->state = STATE_NONCE_SETUP;
				reply->flags = pair->state;
				reply->cmd = CMD_NONCE_REQUEST;
				RETVAL = CMD_SEND_REPLY_TRUE;				
			}
		case CMD_PUBLICKEY_REQUEST:
			// FIXME: check UUIDs, Request PublicKey
			if  ( pair->state == STATE_PUBLICKEY_SETUP  ) {
				reply->flags = pair->hasPublicKey;
				reply->cmd = CMD_PUBLICKEY_SEND;
				memcpy( &(reply->publickey), &(config->publickey), crypto_box_PUBLICKEYBYTES );
				pair->state = STATE_PUBLICKEY_SETUP;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
		case CMD_PUBLICKEY_SEND:
			if  ( pair->state == STATE_PUBLICKEY_SETUP ) {
				memcpy( &(pair->publickey), &(cp->publickey), crypto_box_PUBLICKEYBYTES );
				pair->hasPublicKey = 1;
				#ifdef HAVE_LOCAL_CRYPTO
				curve25519_donna((unsigned char *)&pair->sharedkey, (unsigned char *)&config->privatekey, (unsigned char *)&pair->publickey);
				if (config->debug) {
					printf ( "   %24s: ", "Shared Key" );
					util_print_bin2hex((unsigned char *)pair->sharedkey, crypto_box_PUBLICKEYBYTES);
					printf ( "   %24s: ", "private Key" );
					util_print_bin2hex((unsigned char *)config->privatekey, crypto_box_PUBLICKEYBYTES);
					printf ( "   %24s: ", "public key" );
					util_print_bin2hex((unsigned char *)pair->publickey, crypto_box_PUBLICKEYBYTES);
				}
				
				#endif					

				// flags contains hasPublicKey? of remote 
				if ( cp->flags == 1 ) {
					reply->flags = pair->hasPublicKey;
					reply->cmd = CMD_PUBLICKEY_ACK;
					pair->state = STATE_NONCE_SETUP;
				} else {
					reply->flags = pair->hasPublicKey;
					reply->cmd = CMD_PUBLICKEY_SEND;
					memcpy( &(reply->publickey), &(config->publickey), crypto_box_PUBLICKEYBYTES );
				}
				RETVAL = CMD_SEND_REPLY_TRUE;
			}	
		break;
		
		case CMD_PUBLICKEY_RESET:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_VERIFY_SYN:
			if  ( ( pair->state == STATE_CONFIGURED  )  || ( pair->state == STATE_CONFIGURED_SYN  ) || ( pair->state == STATE_CONFIGURED_SYNACK  ) )
			{
				reply->flags = 0;
				reply->cmd = CMD_VERIFY_SYNACK;
				pair->state = STATE_CONFIGURED_SYN;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}		
		break;
		case CMD_VERIFY_SYNACK:
			if  ( 
				( pair->state == STATE_CONFIGURED  )  || 
				( pair->state == STATE_CONFIGURED_SYN  ) || 
				( pair->state == STATE_CONFIGURED_SYNACK  ) 
			)
			{
				reply->flags = 0;
				reply->cmd = CMD_VERIFY_ACK;
				pair->state = STATE_CONFIGURED_SYNACK;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
		case CMD_VERIFY_ACK:
			if  ( ( pair->state == STATE_CONFIGURED  )  || ( pair->state == STATE_CONFIGURED_SYN  ) || ( pair->state == STATE_CONFIGURED_SYNACK  ) )
			{
				pair->state = STATE_PAIRED;
				reply->flags = 0;
				reply->cmd = CMD_VERIFY_ACK;
				pair->state = STATE_PAIRED;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
				
		case CMD_APP_MESSAGE:
			if  (  pair->state == STATE_PAIRED  ) {
				if ( cp->flags == CMD_SEND_REPLY_TRUE ) {
					RETVAL = CMD_SEND_REPLY_TRUE;
				} else {
					RETVAL = CMD_SEND_REPLY_FALSE;		
				}
				reply->cmd = CMD_APP_MESSAGE_ACK;
			}
		break;
		case CMD_APP_MESSAGE_ACK:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_APP_REGISTER:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		
		case CMD_APP_LIST:
			if  (  pair->state == STATE_PAIRED  ) {
				RETVAL = CMD_SEND_REPLY_TRUE;
//				a->action = (uint32_t )ACTION_READ;
//				a->action_control = 0;
//				a->action_value = 0;
				reply->controlers = config->controlers;
				reply->numOfControllers = config->numOfControllers;
				
				reply->cmd = CMD_APP_LIST_ACK;
			}				
		break;
		
		case CMD_APP_LIST_ACK:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		
		case CMD_CONFIG_MESSAGE:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_CONFIG_DELIGATE:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_CONFIG_PAIR:
				RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_SEND_REPLY_FALSE:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;
		case CMD_SEND_REPLY_TRUE:
			RETVAL = CMD_SEND_REPLY_FALSE;
		break;		
		default:
			reply->flags = 0;
			reply->cmd = CMD_INVAILD_CMD;
			RETVAL = CMD_SEND_REPLY_TRUE;
		break;

	}

//#ifndef ESP8266	
	if ( previous_state.state == pair->state ) 
	{
		CHATFABRIC_DEBUG_FMT(config->debug,
			"[DEBUG][%s:%s:%d]  === [STATE] %-16s ==> %-16s %-36s \n", 
			__FILE__, __FUNCTION__, __LINE__,
			stateLookup(previous_state.state), stateLookup(pair->state), "STATE WAS UNCHANGED"  );
	} else {
		if ( pair->state == STATE_PAIRED ) {
#ifdef ESP8266
			config->pairfile = "1";
#endif
			config->hasPairs = 1;
			CHATFABRIC_DEBUG(config->debug, " Running Pair Config" );
			chatFabric_pairConfig(config, pair, 1 );
		}
		CHATFABRIC_DEBUG_FMT(config->debug,
			"[DEBUG][%s:%s:%d]  === [STATE] %-16s ==> %-16s %-36s \n",
			__FILE__, __FUNCTION__, __LINE__,
			stateLookup(previous_state.state), stateLookup(pair->state), "!!! STATE CHANGED !!" );
	}
//#endif

	*replyCmd = RETVAL;
	*e = ERROR_OK;
	CHATFABRIC_DEBUG(config->debug, " return" );
}
