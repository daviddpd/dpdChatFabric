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

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ESP8266
#include "esp8266.h"
//#define __error (void)0
#endif

#ifdef ESP8266

void CP_ICACHE_FLASH_ATTR
print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		os_printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { os_printf (" "); }
	}
	os_printf ("\n");

}

void CP_ICACHE_FLASH_ATTR
chatFabric_hexprint ( unsigned char *str, uint32_t len ){
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
chatFabric_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght ) {
	char str[5] = { '0', 'x', '0', '0' , 0 };
	int i=0,x=0;
	uint32_t dst_int;
	for ( i = 0; i<hexLength; i=i+2) {
		str[2] = hex[i];
		str[3] = hex[i+1];
		if ( x < dstLenght ) {
			os_sprintf (str, "%d", &dst_int );
			dst_int = strtoul(str, NULL, 16);
			dst[x] = (unsigned char)dst_int;
		}
		x++;
	}

}

void CP_ICACHE_FLASH_ATTR
chatFabric_usage(char *p) {
	return;
}

void CP_ICACHE_FLASH_ATTR
chatFabric_args(int argc, char**argv, chatFabricConfig *config) {
	return;
}
#else
void CP_ICACHE_FLASH_ATTR
print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { printf (" "); }
	}
	printf ("\n");

}

void CP_ICACHE_FLASH_ATTR
chatFabric_hexprint ( unsigned char *str, uint32_t len ){
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
			printf ( "\n %4s: ", ' ' );		
		}
	}

	printf ("\n");

}

void CP_ICACHE_FLASH_ATTR
chatFabric_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght ) {
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
void CP_ICACHE_FLASH_ATTR
chatFabric_usage(char *p) {
	printf ("%s -config configfile\n", p);
	printf ("   -c --config   FILE           config file to use.\n");
	printf ("   -P --pairfile FILE           pairing binary data.\n");
	printf ("   -i --ip IPADDRESS           ip address.\n");
	printf ("   -d --debug                  ip address.\n");
	printf ("   -p --port                   port number.\n");
	printf ("   -k --genkeys                generate public/private key pair.\n");

	printf ("      --to0                    Namespace UUID, Send TO (uuid0).\n");
	printf ("      --to1                    Instance UUID, Send TO (uuid1).\n");

	printf ("   -u --uuid0                  Namespace UUID (uuid0).\n");
	printf ("   -v --uuid1                  Instance UUID (uuid1).\n");

	printf ("   -z --genuuid1               generate uuid1, set uuid0 to NIL/zeros.\n");
	printf ("   -w --writeconfig FILE       new configuration file to write\n");
	printf ("   -m --message STRING         Send message in payload.\n");

	return;
}

void CP_ICACHE_FLASH_ATTR
chatFabric_args(int argc, char**argv, chatFabricConfig *config) {
	int ch;
	uint32_t status;
	static const unsigned char basepoint[32] = {9};

	config->configfile = NULL;
	config->newconfigfile = NULL;
	config->pairfile = NULL;
	config->ip = NULL;
	config->port = 32000;	
	config->debug = 0;
	config->writeconfig = 0;
	
	static struct option longopts[] = {
		{	"config",	required_argument,	NULL,	'c'	},
		{	"pairfile",		required_argument,	NULL,	'P'	},
		{	"debug",	no_argument,		NULL,	'd'	},
		{	"ip",		required_argument,	NULL,	'i'	},
		{	"port",		required_argument,	NULL,	'p'	},
		{	"genkeys",	no_argument,		NULL,	'k'	},

		{	"to0",	required_argument,	NULL,	'a'	},
		{	"to1",	required_argument,	NULL,	'b'	},

		{	"uuid0",	required_argument,	NULL,	'u'	},
		{	"uuid1",	required_argument,	NULL,	'v'	},

		{	"genuuid1",	no_argument,		NULL,	'z'	},
		{	"writeconfig",	required_argument,		NULL,	'w'	},
		{	"message",	required_argument,		NULL,	'm'	},
		

		/*  remember a zero line, else 
			getopt_long segfaults with unknown options */
	    {NULL, 			0, 					0, 		0	}, 
			
	};

	
	while ((ch = getopt_long(argc, argv, "a:b:c:di:kp:s:u:v:zw:P:m:", longopts, NULL)) != -1) {
		switch (ch) {

			case 'a':
				uuid_from_string(
					optarg, 
					&(config->to.u0), 
					&status);
			break;
			case 'b':
				uuid_from_string(
					optarg, 
					&(config->to.u1), 
					&status);	
			break;

			case 'c':
				//printf ( "Arg --config : Value : %s \n", optarg );
				config->configfile = optarg;
			break;
			case 'd':
				//printf ( "Arg --debug : Value : %s \n", optarg );
				config->debug = 1;
			break;
			case 'i':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->ip = optarg;
			break;
			case 'p':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->port = atoi(optarg);
			break;
			case 'P':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->pairfile =	optarg;
			break;
			case 'u':
				uuid_from_string(
					optarg, 
					&(config->uuid.u0), 
					&status);
			break;
			case 'v':
				uuid_from_string(
					optarg, 
					&(config->uuid.u1), 
					&status);			
			break;
			case 'k':
				#ifdef HAVE_SODIUM			
				crypto_box_keypair((unsigned char *)&(config->publickey), (unsigned char *)&(config->privatekey));
				#endif 
				#ifdef HAVE_LOCAL_CRYPTO
				arc4random_buf((unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES);
				curve25519_donna((unsigned char *)&config->publickey, (unsigned char *)&config->privatekey, (unsigned char *)&basepoint);
				#endif

			break;
			case 'z':
				uuid_create_nil(&(config->uuid.u0),  &status);
				uuid_create(&(config->uuid.u1),  &status);
			break;
			case 'w':
				//printf ( "Arg --config : Value : %s \n", optarg );
				config->newconfigfile = optarg;
				config->writeconfig = 1;				
			break;			
			case 'm':
				//printf ( "Arg --ip : Value : %s \n", optarg );
				config->msg = (unsigned char *)optarg;
			break;
			case '?':
			case 'h':
			default:
				chatFabric_usage(argv[0]);
			break;
		}
	}
	argc -= optind;
	argv += optind;


}
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
	unsigned char c,t;
//	enum chatFabricConfigTags t;
#ifdef ESP8266

// FIXME:  Add if reconfig condition ... from button/GPIO ?
	CHATFABRIC_DEBUG_FMT(config->debug,  
		"[DEBUG][%s:%s:%d] Reading in flash\n", 
		__FILE__, __FUNCTION__, __LINE__ );
		
	print_bin2hex((unsigned char *)&flashConfig, 256);	

	CHATFABRIC_DEBUG_FMT(config->debug,  
		"[DEBUG][%s:%s:%d] Readed in flash\n", 
		__FILE__, __FUNCTION__, __LINE__ );

	if ( system_param_load (CP_ESP_PARAM_START_SEC, 0, &(flashConfig), 4096) == FALSE ) {
		CHATFABRIC_DEBUG_FMT(config->debug, "Read from flash failed." ); 	
	}


	print_bin2hex((unsigned char *)&flashConfig, 256);	
	print_bin2hex((unsigned char *)&flashConfig+2048, 256);	

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
		config->configfile = NULL;
		config->hasPairs = 0;
		config->pairfile = NULL;
		
		
		filesize=0;
		// These are created in args, so for the embedded solution
		// putting here.
		uuid_create( &(config->to.u0), &status);
		uuid_create( &(config->to.u1), &status);
		uuid_create_nil( &(config->uuid.u0), &status);
		uuid_create( &(config->uuid.u1), &status);
	
		config->debug = 1 & config->debug;
		arc4random_buf((unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES);

		curve25519_donna((unsigned char *)&config->publickey, (unsigned char *)&config->privatekey, (unsigned char *)&basepoint);

	}
	str = &(flashConfig[0]);
	
#endif

	
	if ( config->configfile != NULL ) 
	{
#ifndef ESP8266
			bzero(&fs, sizeof(fs));		
			stat(config->configfile, &fs);
			fp = fopen(config->configfile,"r");	
			if ( fp == NULL ) {
				fprintf(stderr, " Error, can't open file %s \n", config->configfile );			
			}
			str=(unsigned char *)calloc(fs.st_size,sizeof(unsigned char));
			fread(str, sizeof (unsigned char), fs.st_size, fp );
			filesize=fs.st_size;
#endif
			
			i=0;
			
			while (i<filesize) 
			{
				memcpy(&t, str+i, 1);
				CHATFABRIC_DEBUG_FMT(config->debug,  
					"[DEBUG][%s:%s:%d] Parsing File %02x %4d\n", 
					__FILE__, __FUNCTION__, __LINE__, t, i);
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
						uuid_dec_be(str+i, &config->uuid.u0);
						i+=16;
					break;		
					case cftag_uuid1:			// 1+16
						uuid_dec_be(str+i, &config->uuid.u1);
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

			chatPacket_tagDataEncoder ( CP_INT32, str, &i, cftag_header, 0, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, &i, cftag_configLength, len, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, &i, cftag_hasPairs, config->hasPairs, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_UUID, str, &i, cftag_uuid0, 0, NULL, 0,  &config->uuid.u0);
			chatPacket_tagDataEncoder ( CP_UUID, str, &i, cftag_uuid1, 0, NULL, 0,  &config->uuid.u1);
			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_publickey, 0,(unsigned char *)&(config->publickey), crypto_box_PUBLICKEYBYTES, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_privatekey, 0,(unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES, NULL);

#ifdef ESP8266
		if ( system_param_save_with_protect (CP_ESP_PARAM_START_SEC, &(flashConfig[0]), 4096) == FALSE ) {
			CHATFABRIC_DEBUG_FMT(1,  
				"[DEBUG][%s:%s:%d] Failed to Save Config to Flash\n", 
				__FILE__, __FUNCTION__, __LINE__ );
		} else {
			CHATFABRIC_DEBUG_FMT(1,  
				"[DEBUG][%s:%s:%d] Save Succesful.\n", 
				__FILE__, __FUNCTION__, __LINE__ );		
			print_bin2hex((unsigned char *)&flashConfig, 256);	
				
		}

#else
			fwrite (str, sizeof (unsigned char), len, fp );
			fclose(fp);
			free(str);
#endif
		}
	}

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
	unsigned char c, t;
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
			
			chatPacket_tagDataEncoder ( CP_INT32, str, &i, cftag_header, 0, NULL, 0, NULL);			
			chatPacket_tagDataEncoder ( CP_INT32, str, &i, cftag_pairs, 1, NULL, 0, NULL);
			chatPacket_tagDataEncoder ( CP_INT32, str, &i, cftag_pairLength, len, NULL, 0, NULL);
			CHATFABRIC_DEBUG_FMT(1,
				"[DEBUG][%s:%s:%d] Encoding Pair Config Length %d\n",
				__FILE__, __FUNCTION__, __LINE__, len );

			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_hasPublicKey, 0, (unsigned char *)&pair->hasPublicKey, 1, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_hasNonce, 0, (unsigned char *)&pair->hasNonce, 1, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_state, 0, (unsigned char *)&pair->state, 1, NULL);

			chatPacket_tagDataEncoder ( CP_INT32, str, &i, cftag_serial, pair->serial, NULL, 0, NULL);

			chatPacket_tagDataEncoder ( CP_UUID, str, &i, cftag_uuid0, 0, NULL, 0,  &pair->uuid.u0);
			chatPacket_tagDataEncoder ( CP_UUID, str, &i, cftag_uuid1, 0, NULL, 0,  &pair->uuid.u1);						

			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_publickey, 0,(unsigned char *)&(pair->publickey), crypto_box_PUBLICKEYBYTES, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_mynonce, 0, (unsigned char *)&(pair->mynonce), crypto_secretbox_NONCEBYTES, NULL);
			chatPacket_tagDataEncoder ( CP_DATA8, str, &i, cftag_nonce, 0, (unsigned char *)&(pair->nonce), crypto_secretbox_NONCEBYTES, NULL);
#ifdef ESP8266
		if ( system_param_save_with_protect (CP_ESP_PARAM_START_SEC, &(flashConfig[0]), 4096) == FALSE ) {
			CHATFABRIC_DEBUG_FMT(1,
				"[DEBUG][%s:%s:%d] Failed to Save Config to Flash\n",
				__FILE__, __FUNCTION__, __LINE__ );
		} else {
			CHATFABRIC_DEBUG_FMT(1,  
				"[DEBUG][%s:%s:%d] Save Succesful.\n", 
				__FILE__, __FUNCTION__, __LINE__ );		
		}

#else
			fwrite (str, sizeof (unsigned char), len, fp );
			fclose(fp);
#endif
		} else {
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
				CHATFABRIC_DEBUG_FMT(config->debug,
					"[DEBUG][%s:%s:%d] Parsing File %02x %4d\n",
					__FILE__, __FUNCTION__, __LINE__, t, i);
				
				++i;			
				switch (t){
					case cftag_header:
						i+=4;
					break;					
					case cftag_pairLength:
						memcpy(&ni, str+i, 4);
						i+=4;
						filesize = 2048 + ntohl(ni);
					CHATFABRIC_DEBUG_FMT(config->debug,  
						"[DEBUG][%s:%s:%d] Got Length as  %02x %08x %4d\n", 
						__FILE__, __FUNCTION__, __LINE__, t, ni, filesize);
					break;
					case cftag_pairs:
						memcpy(&ni, str+i, 4);
						i+=4;
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
						uuid_dec_be(str+i, &pair->uuid.u0);
						i+=16;
					break;		
					case cftag_uuid1:			// 1+16
						uuid_dec_be(str+i, &pair->uuid.u1);
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
chatFabric_consetup( chatFabricConnection *c,  char *ip, int port, int doBind )
{

	int lowwater = 64;
	c->socket=socket(AF_INET,SOCK_DGRAM,0);
	bzero( &(c->sockaddr),sizeof(c->sockaddr) );
	c->sockaddr.sin_family = AF_INET;
	if ( ip == 0 ) {
		c->sockaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	} else {
		c->sockaddr.sin_addr.s_addr=inet_addr(ip);
	}
	c->sockaddr.sin_port=htons(port);
	setsockopt(c->socket, SOL_SOCKET, SO_RCVLOWAT, &lowwater, sizeof(lowwater));

	if ( doBind ==  1 ) {
		bind(c->socket,(struct sockaddr *)&c->sockaddr,sizeof(c->sockaddr));
	}
	
} 

#endif


enum chatFabricErrors CP_ICACHE_FLASH_ATTR
chatFabric_controller(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config,  msgbuffer *b) 
{
	int n, buffersize=1460;
	unsigned char *mesg;
	socklen_t len;
	chatPacket *cp;
	msgbuffer mb;
	unsigned char * nullmsg = 0;
	b->length = 0;		
	enum chatFabricErrors e;
#ifndef ESP8266
	
	if ( c->socket < 1 ) {
		 chatFabric_consetup(c, config->ip, config->port, 0 );
		if ( c->socket == -1 ) {
			CHATFABRIC_DEBUG(config->debug, "chatFabric connection setup failed" );
			return ERROR_SOCKET;
		}	
	}
#endif

	if ( pair->state != STATE_PAIRED ) {
		cp = chatPacket_init (config, pair, CMD_PAIR_REQUEST,  nullmsg, 0,  0);
		chatPacket_encode ( cp, config, pair,  &mb, _CHATPACKET_ENCRYPTED, COMMAND);
	} else {	
		cp = chatPacket_init (config, pair, CMD_APP_MESSAGE,  config->msg, strlen((const char *)config->msg),  CMD_SEND_REPLY_TRUE);
		chatPacket_encode ( cp, config, pair,  &mb, _CHATPACKET_ENCRYPTED, DATA);
	}

#ifndef ESP8266

	len = sizeof(c->sockaddr);
	n = sendto(c->socket, mb.msg, mb.length, 0, (struct sockaddr *)&(c->sockaddr), len);
#endif

	if ( config->debug )
		chatPacket_print(cp, OUT);
						
//#ifndef ESP8266
	free(mb.msg);
//#endif
	mb.length = 0;
	chatPacket_delete(cp);
	
	do { 
		e = chatFabric_device(c, pair, config, b);
	} while ( (e == ERROR_OK) && (pair->state != STATE_PAIRED)  );
	
	return e;
	
}

enum chatFabricErrors CP_ICACHE_FLASH_ATTR
chatFabric_device(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config,  msgbuffer *b) 
{
	CHATFABRIC_DEBUG(config->debug, " Start " );

	int n, buffersize=1460;
	unsigned char *mesg;
	socklen_t len;
	chatPacket *cp, *cp_reply;
	msgbuffer mb;
	b->length = 0;		
	uint32_t heap;
	

	enum chatPacketCommands replyCmd;
	enum chatPacketPacketTypes cptype;
	enum chatFabricErrors e;
	
#ifdef ESP8266
	n = mbuff.length;
	mesg = mbuff.msg;
	int errno = 0;
#else
	if ( c->socket < 1 ) {
		 chatFabric_consetup(c, config->ip, config->port, 1 );
		if ( c->socket == -1 ) {
			CHATFABRIC_DEBUG(config->debug, "chatFabric connection setup failed" );
			return ERROR_SOCKET;
		}	
	}

	mesg=(unsigned char *)calloc(buffersize,sizeof(unsigned char));

	CHATFABRIC_DEBUG(config->debug, "Waiting for Packet.\n\n" );

	len = sizeof(c->sockaddr);

	n = recvfrom(c->socket,mesg,buffersize,0,(struct sockaddr *)&(c->sockaddr),&len);
#endif 	
	if ( n == -1 ) {
#ifndef ESP8266
		free(mesg); /* needed */ 
#endif
		CHATFABRIC_DEBUG(config->debug, "recvfrom returned an error" );
		CHATFABRIC_DEBUG_FMT(config->debug,  
			"[DEBUG][%s:%s:%d] ERRNO: %d \n", 
			__FILE__, __FUNCTION__, __LINE__,  errno );
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
	

//		if ( config->debug )
//			chatPacket_print (cp, IN);

		if ( cp->payloadLength > 0 ) 
		{
			b->length = cp->payloadLength;
			b->msg=(unsigned char*)calloc(b->length,sizeof(unsigned char));		
			memcpy(b->msg, cp->payload, b->length);
		} else {
			b->length = 0;
		}
#ifndef ESP8266		
		free(mesg);
#endif
	}
	
	CHATFABRIC_DEBUG(config->debug, " == cp init0 " );
	cp_reply = chatPacket_init0 ();
	
	CHATFABRIC_DEBUG(config->debug, " == starting state machine == \n " );
	stateMachine ( config, cp, pair, cp_reply, &replyCmd, &e);
	if ( replyCmd == CMD_SEND_REPLY_TRUE ) {

//		if ( config->debug )
//			chatPacket_print (cp_reply, OUT);

		switch (cp_reply->cmd) {
			case CMD_NONCE_SEND:
				cptype = NONCE;
			break;
			case CMD_PUBLICKEY_SEND:
				cptype = PUBLICKEY;
			break;
			default:
				cptype = COMMAND;
			break;
		}
		chatPacket_encode ( cp_reply, config, pair, &mb, _CHATPACKET_ENCRYPTED, cptype);
		// FIXME: Error check and handle sending error
#ifdef ESP8266
		espconn_sent(&c->conn, (uint8 *)mb.msg, mb.length);
#else
		n = sendto(c->socket, mb.msg, mb.length, 0, (struct sockaddr *)&(c->sockaddr), len);
#endif
		CHATFABRIC_DEBUG(config->debug, "chatPacket UDP data sent." );

//#ifndef ESP8266
		free(mb.msg);
//#endif
		mb.length = 0;
		chatPacket_delete(cp_reply);
	} else {
		chatPacket_delete(cp_reply);	
	}
	
	chatPacket_delete(cp);
	CHATFABRIC_DEBUG(config->debug, "function return." );
	return ERROR_OK;
}



void CP_ICACHE_FLASH_ATTR
stateMachine (chatFabricConfig *config, chatPacket *cp, chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd, enum chatFabricErrors *e)
{	
	CHATFABRIC_DEBUG(config->debug, " Start " );

	chatFabricPairing  previous_state;
	uint32_t status;
	
	enum chatPacketCommands RETVAL;
	
	if (  
		( uuid_compare(&(cp->to.u0), &(config->uuid.u0), &status) == 0 ) &&
		( uuid_compare(&(cp->to.u1), &(config->uuid.u1), &status) == 0  )
	) {
		*e = ERROR_INVAILD_DEST;
	}

	previous_state.state =  pair->state;
	previous_state.hasPublicKey = pair->hasPublicKey;

#ifndef ESP8266
	reply->to.u0 = cp->from.u0;
	reply->to.u1 = cp->from.u1;
	reply->from.u0 = config->uuid.u0;
	reply->from.u1= config->uuid.u1;
#else

	memcpy(&reply->to.u0, &cp->from.u0, 16 );
	memcpy(&reply->to.u1, &cp->from.u1, 16 );
	memcpy(&reply->from.u0, &config->uuid.u0, 16 );
	memcpy(&reply->from.u1, &config->uuid.u1, 16 );

#endif
	
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
				pair->uuid.u0 = cp->from.u0;
				pair->uuid.u1 = cp->from.u1;
				RETVAL = CMD_SEND_REPLY_TRUE;	
			}
		break;
		case CMD_PAIR_REQUEST_ACK:
			if ( pair->state == STATE_UNCONFIGURED ) {
				reply->flags = 0;
				reply->cmd = CMD_PUBLICKEY_REQUEST;
				pair->state = STATE_PUBLICKEY_SETUP;
				pair->uuid.u0 = cp->from.u0;
				pair->uuid.u1 = cp->from.u1;
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
					RETVAL = CMD_SEND_REPLY_FALSE;
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
					print_bin2hex((unsigned char *)pair->sharedkey, crypto_box_PUBLICKEYBYTES);
					printf ( "   %24s: ", "private Key" );
					print_bin2hex((unsigned char *)config->privatekey, crypto_box_PUBLICKEYBYTES);
					printf ( "   %24s: ", "public key" );
					print_bin2hex((unsigned char *)pair->publickey, crypto_box_PUBLICKEYBYTES);
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
