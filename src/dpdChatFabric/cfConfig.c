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

#include "cfConfig.h"
#include "util.h"
#ifdef ESP8266
#include "esp8266.h"
#endif

extern hostmeta_t hostMeta;

void CP_ICACHE_FLASH_ATTR
cfConfigInit(chatFabricConfig *config) {

	config->configfile = NULL;
	config->newconfigfile = NULL;
	config->pairfile = NULL;
	config->ip = NULL;
	config->msg = NULL;
	config->controlers = NULL;
	config->numOfControllers = -1;
	config->callback = NULL;
#ifdef ESP8266
	config->mode = SOFTAP_MODE;
#else
    config->mode = 0;
#endif
    
/*	
exp1            1021/tcp    # RFC3692-style Experiment 1 (*)    [RFC4727]
exp1            1021/udp    # RFC3692-style Experiment 1 (*)    [RFC4727]
exp2            1022/tcp    # RFC3692-style Experiment 2 (*)    [RFC4727]
exp2            1022/udp    # RFC3692-style Experiment 2 (*)    [RFC4727]
*/
	config->port = 2030;
	config->type = -1; // SOCK_STREAM SOCK_DGRAM SOCK_RAW SOCK_RDM SOCK_SEQPACKET
	config->hasPairs = 0;

	uuuid2_gen_nil( &(config->uuid.u0));
	uuuid2_gen( &(config->uuid.u1));

	config->debug = 1;
	config->writeconfig = 1;

//	config->defaulthostname = NULL:
	bzero(&(config->hostname), 33);

    if ( hostMeta.status ) {
        CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "Using hostMeta Name:  %s ", hostMeta.hostname );
        memcpy((char*)&(config->hostname), (char*)hostMeta.hostname, strlen(hostMeta.hostname));
    }
//  172.16.0.0 
#ifdef ESP8266
	struct ip_info info;
	
	IP4_ADDR(&info.ip, 172,16,250,1); // 1+4
	IP4_ADDR(&info.netmask, 255,255,255,0); // 1+4
	config->ap_ipv4 = info.ip.addr; // 1+4
	config->ap_ipv4netmask = info.netmask.addr; // 1+4
	config->sta_ipv4netmask = info.netmask.addr; // 1+4
	IP4_ADDR(&info.ip, 192,168,100,2); // 1+4
	config->sta_ipv4 = info.ip.addr; // 1+4
	IP4_ADDR(&info.ip, 192,168,100,1); // 1+4
	config->ntpv4 = config->sta_ipv4; // 1+4
		
#else
	config->ap_ipv4 = inet_addr("172.16.250.1"); // 1+4
	config->ap_ipv4netmask = inet_addr("255.255.255.0");; // 1+4
	config->sta_ipv4 = inet_addr("192.168.100.2"); // 1+4
	config->sta_ipv4netmask = inet_addr("255.255.255.0");; // 1+4
	config->ntpv4 = inet_addr("192.168.100.1"); // 1+4

#endif 
	config->ap_ipv4gw  = config->ap_ipv4; // 1+4
	config->ap_ipv4ns1 = config->ap_ipv4; // 1+4
	config->ap_ipv4ns2 = config->ap_ipv4; // 1+4	

	config->sta_ipv4gw  = config->sta_ipv4; // 1+4
	config->sta_ipv4ns1 = config->sta_ipv4; // 1+4
	config->sta_ipv4ns2 = config->sta_ipv4; // 1+4	

	bzero ((char*)&(config->sta_ipv6), 16);
	bzero ((char*)&(config->sta_ipv6netmask), 16);
	bzero ((char*)&(config->sta_ipv6gw), 16);
	bzero ((char*)&(config->sta_ipv6ns1), 16);
	bzero ((char*)&(config->sta_ipv6ns2), 16);
	bzero ((char*)&(config->ntpv6), 16);

	bzero ((char*)&(config->ap_ipv6), 16);
	bzero ((char*)&(config->ap_ipv6netmask), 16);
	bzero ((char*)&(config->ap_ipv6gw), 16);
	bzero ((char*)&(config->ap_ipv6ns1), 16);
	bzero ((char*)&(config->ap_ipv6ns2), 16);


	config->wifi_ap_switch = 1;
	config->wifi_ap_dhcps_switch = 1;
	config->wifi_sta_switch = 0;
	config->dhcp_client_switch = 1;

	bzero ((char*)&(config->wifi_ap_ssid), 33);
	bzero ((char*)&(config->wifi_ap_passwd), 65);
	bzero ((char*)&(config->wifi_sta_ssid), 33);
	bzero ((char*)&(config->wifi_sta_passwd), 65);
	
	static const unsigned char basepoint[32] = {9};
		
	arc4random_buf((unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES);
	curve25519_donna((unsigned char *)&config->publickey, (unsigned char *)&config->privatekey, (unsigned char *)&basepoint);	

	CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "return");
}

void CP_ICACHE_FLASH_ATTR
cfConfigSetFromStr(chatFabricConfig *config, unsigned char* cstr, int cstr_len) {
	_cfConfigRead(config, 1, cstr, cstr_len);
}
   
   
void CP_ICACHE_FLASH_ATTR
cfConfigRead(chatFabricConfig *config) {
	_cfConfigRead(config, 0, NULL, 0);
}

void CP_ICACHE_FLASH_ATTR
_cfConfigRead(chatFabricConfig *config, int fromStr, unsigned char* cstr, int cstr_len) {

	struct stat fs;
	uint32_t ni;
	int len =0, i=0, filesize=0;
	unsigned char *str;
	unsigned char t;


#ifdef ESP8266
	int fp=0;
	CHATFABRIC_DEBUG_FMT(config->debug, "Starting Config Read, fromStr = %d", fromStr ); 

	if ( !fromStr ) {

		CHATFABRIC_DEBUG_B2H(config->debug, "Config String, RAM, preload   :",
			(unsigned char*)&(flashConfig[0]), 32  );

		if ( system_param_load (CP_ESP_PARAM_START_SEC, 0, &(flashConfig[0]), 4096) == FALSE ) {
			CHATFABRIC_DEBUG(config->debug, "Read from flash failed." ); 
			return;
		}

		CHATFABRIC_DEBUG_B2H(config->debug, "Config String, RAM, post-load :", 
			(unsigned char*)&(flashConfig[0]), 32  );
	}

	if ( fromStr ) {
		filesize = cstr_len;
		str = cstr;
	} else if ( flashConfig[0] == cftag_header ) {
		filesize=4096;	
		config->configfile = "1";
		str = &(flashConfig[0]);
	} else {
		CHATFABRIC_DEBUG_FMT(config->debug, "========== NO CONFIGUTATION DATA FOUND : %02x %02x ==========",flashConfig[0], cftag_header  ); 
		return;
	}
#else

	FILE *fp=0;

	if ( fromStr ) {
		filesize = cstr_len;
		str = cstr;
	} else {
		if ( config->configfile == NULL ) 
		{
			return;		
		}
		bzero(&fs, sizeof(fs));
		fp = fopen(config->configfile,"r");	
		if ( fp == NULL ) {
			return;
		} else {
			stat(config->configfile, &fs);
			str=(unsigned char *)calloc(fs.st_size,sizeof(unsigned char));
			fread(str, sizeof (unsigned char), fs.st_size, fp );
			filesize=fs.st_size;
			fclose(fp);		
		}
	}
#endif
			
	i=0;

//    CHATFABRIC_DEBUG_FMT(config->debug, "Filesize: %d  Index : %d ", filesize, i);

	while (i<filesize) 
	{
		memcpy(&t, str+i, 1);
		++i;
//        CHATFABRIC_DEBUG_FMT(config->debug, "Filesize: %d; Index: %d; Tag %02x ", filesize, i, t);
					
		switch (t){
			case cftag_header:
				i+=4;
			break;			
			case cftag_configLength:
				memcpy(&ni, str+i, 4);
				i+=4;
				filesize = ntohl(ni);
			break;
			case cftag_hasPairs:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->hasPairs = ntohl(ni);
			break;
			case cftag_debug:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->debug = ntohl(ni);
			break;
			case cftag_uuid0:			// 1+16
				memcpy(&config->uuid.u0, str+i, 16);
				i+=16;
			break;		
			case cftag_uuid1:			// 1+16
				memcpy(&config->uuid.u1, str+i, 16);
				i+=16;
			break;		
			case cftag_mode:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->mode = ntohl(ni);
			break;
			case cftag_wifi_ap:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->wifi_ap_switch = ntohl(ni);
			break;
			case cftag_wifi_ap_dhcps:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->wifi_ap_dhcps_switch = ntohl(ni);
			break;
			case cftag_wifi_sta:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->wifi_sta_switch = ntohl(ni);
			break;
			case cftag_wifi_sta_dhcpc:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->dhcp_client_switch = ntohl(ni);
			break;			
			case cftag_dhcps_range_low:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->dhcps_range_low = ntohl(ni);
			break;
			case cftag_dhcps_range_high:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->dhcps_range_high = ntohl(ni);
			break;

			case cftag_sta_ipv4:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->sta_ipv4 = ntohl(ni);
			break;						
			case cftag_sta_ipv4netmask:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->sta_ipv4netmask = ntohl(ni);
			break;
			case cftag_sta_ipv4gw:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->sta_ipv4gw = ntohl(ni);
			break;
			case cftag_sta_ipv4ns1:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->sta_ipv4ns1 = ntohl(ni);
			break;
			case cftag_sta_ipv4ns2:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->sta_ipv4ns2 = ntohl(ni);
			break;
			case cftag_sta_ntpv4:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->ntpv4 = ntohl(ni);
			break;
				
			case cftag_ap_ipv4:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->ap_ipv4 = ntohl(ni);
			break;						
			case cftag_ap_ipv4netmask:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->ap_ipv4netmask = ntohl(ni);
			break;
			case cftag_ap_ipv4gw:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->ap_ipv4gw = ntohl(ni);
			break;
			case cftag_ap_ipv4ns1:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->ap_ipv4ns1 = ntohl(ni);
			break;
			case cftag_ap_ipv4ns2:
				memcpy(&ni, str+i, 4);
				i+=4;
				config->ap_ipv4ns2 = ntohl(ni);
			break;

			case cftag_sta_ipv6:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->sta_ipv6), str+i, 16);
				i+=16;
			break;		
			case cftag_sta_ipv6netmask:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->sta_ipv6netmask), str+i, 16);
				i+=16;
			break;		
			case cftag_sta_ipv6gw:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->sta_ipv6gw), str+i, 16);
				i+=16;
			break;		
			case cftag_ap_ipv6ns1:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->ap_ipv6ns1), str+i, 16);
				i+=16;
			break;		
			case cftag_ap_ipv6ns2:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->ap_ipv6ns2), str+i, 16);
				i+=16;
			break;		

			case cftag_hostname:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->hostname), str+i, 32);
				i+=32;
			break;		
			case cftag_wifi_ap_ssid:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->wifi_ap_ssid), str+i, 32);
				i+=32;
			break;		
			case cftag_wifi_ap_passwd:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->wifi_ap_passwd), str+i, 64);
				i+=64;
			break;		
			case cftag_wifi_sta_ssid:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->wifi_sta_ssid), str+i, 32);
				i+=32;
			break;		
			case cftag_wifi_sta_passwd:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->wifi_sta_passwd), str+i, 64);
				i+=64;
			break;								
			case cftag_publickey:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->publickey), str+i, crypto_box_PUBLICKEYBYTES);
				i+=crypto_box_PUBLICKEYBYTES;
			break;
			case cftag_privatekey:	// 1+crypto_box_SECRETKEYBYTES
				memcpy(&(config->privatekey), str+i, crypto_box_SECRETKEYBYTES);
				i+=crypto_box_SECRETKEYBYTES;
			break;
			case cftag_eof:
				filesize=i;
			break;				
			default:
// 				CHATFABRIC_DEBUG_FMT(config->debug,  
// 					"[DEBUG][%s:%s:%d] Bad Config File Tag : %02x \n", 
// 					__FILE__, __FUNCTION__, __LINE__,  t );
			break;
		}
	}
}


void CP_ICACHE_FLASH_ATTR
cfConfigWrite(chatFabricConfig *config) {

	msgbuffer configstr;
	msgbuffer keys;
	unsigned char ch =(unsigned char)cftag_eof; 
	
	_createConfigString (config, &configstr);
    CHATFABRIC_DEBUG_B2H(config->debug, "Config String", 
    	(unsigned char*)configstr.msg, configstr.length  );

	_createKeyString (config, &keys);
    CHATFABRIC_DEBUG_B2H(config->debug, "Keys String", 
    	(unsigned char*)keys.msg, keys.length  );

	_GLOBAL_DEBUG = config->debug;

#ifdef ESP8266
	memcpy( &(flashConfig[0]), configstr.msg, configstr.length);
	memcpy( &(flashConfig[configstr.length]), keys.msg, keys.length);
	memcpy( &(flashConfig[configstr.length+keys.length]), &ch, 1);
	
    CHATFABRIC_DEBUG_B2H(config->debug, "Config String, RAM, presave   :",
    	(unsigned char*)&(flashConfig[0]), 32  );
	
	if ( system_param_save_with_protect (CP_ESP_PARAM_START_SEC, &(flashConfig[0]), 4096) == FALSE ) {
		CHATFABRIC_DEBUG(config->debug, "ESP Save With Protect Failed.");
	} else {
		CHATFABRIC_DEBUG(config->debug, "ESP Save With Protect - SUCCESS.");	
	}

    CHATFABRIC_DEBUG_B2H(config->debug, "Config String, RAM, postsave  :",
    	(unsigned char*)&(flashConfig[0]), 32  );

#else
	FILE *fp=0;
	fp = fopen(config->newconfigfile,"w");
	if ( fp != 0 ) {  
		size_t fwi = fwrite (configstr.msg, sizeof (unsigned char), configstr.length, fp );
		fwi = fwrite (keys.msg, sizeof (unsigned char), keys.length, fp );
		fwi = fwrite (&ch, sizeof (unsigned char), 1, fp );
		
		fclose(fp);	
	}

#endif

	free(configstr.msg);
	free(keys.msg);


}



void CP_ICACHE_FLASH_ATTR 
_createKeyString (chatFabricConfig *config, msgbuffer *str) 
{
	int len =0, i=0;

	len+=1+crypto_box_PUBLICKEYBYTES;
	len+=1+crypto_box_SECRETKEYBYTES;
	
	str->msg =  (unsigned char *)calloc(len,sizeof(unsigned char));
	str->length = len;

    CHATFABRIC_DEBUG_B2H(config->debug, "Keys Public", 
    	(unsigned char*)&(config->publickey), crypto_box_PUBLICKEYBYTES  );
    CHATFABRIC_DEBUG_B2H(config->debug, "Keys Private", 
    	(unsigned char*)&(config->privatekey), crypto_box_SECRETKEYBYTES  );


	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_publickey, 0,(unsigned char *)&(config->publickey), crypto_box_PUBLICKEYBYTES, NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_privatekey, 0,(unsigned char *)&(config->privatekey), crypto_box_SECRETKEYBYTES, NULL);
	
	
}

void CP_ICACHE_FLASH_ATTR 
_createConfigString (chatFabricConfig *config, msgbuffer *str) 
{

	int len =0, i=0;

	len+=1+4; // header
	len+=1+4; // length
	len+=1+4; // haspairs
	len+=1+4; // debug

	len+=1+16; // uuid
	len+=1+16;

	len+=1+4; // mode

	len+=1+4; // cftag_wifi_ap
	len+=1+4; // cftag_wifi_ap_dhcps
	len+=1+4; // cftag_wifi_sta
	len+=1+4; // cftag_wifi_sta_dhcpc

	len+=1+4; // ipv4 (l)
	len+=1+4; // ipv4 (h)

	len+=1+4; // ipv4
	len+=1+4; // ipv4nm
	len+=1+4; // ipv4gw
	len+=1+4; // ipv4ns1
	len+=1+4; // ipv4ns2
	len+=1+4; // ipv4 ntp

	len+=1+4; // ipv4
	len+=1+4; // ipv4nm
	len+=1+4; // ipv4gw
	len+=1+4; // ipv4ns1
	len+=1+4; // ipv4ns2

	len+=1+16; // ipv66	len+=1+16; // ipv4nm
	len+=1+16; // ipv6 netmask
	len+=1+16; // ipv6gw
	len+=1+16; // ipv6ns1
	len+=1+16; // ipv6ns2
	len+=1+16; // ipv6 ntp

	len+=1+16; // ipv6
	len+=1+16; // ipv6nm
	len+=1+16; // ipv6gw
	len+=1+16; // ipv6ns1
	len+=1+16; // ipv6ns2

	len+=1+32; // hostname

	len+=1+32; // ssid
	len+=1+64; // passwd

	len+=1+32; // ssid
	len+=1+64; // passwd
	
	str->msg =  (unsigned char *)calloc(len,sizeof(unsigned char));
	str->length = len;

	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_header, 0, NULL, 0, NULL);
//	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_configLength, len, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_hasPairs, config->hasPairs, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_debug, config->debug, NULL, 0, NULL);
	cfTagEncoder ( CP_UUID, str->msg, (uint32_t *)&i, cftag_uuid0, 0, NULL, 0,  &config->uuid.u0);
	cfTagEncoder ( CP_UUID, str->msg, (uint32_t *)&i, cftag_uuid1, 0, NULL, 0,  &config->uuid.u1);

	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_mode, config->mode, NULL, 0, NULL);

	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_wifi_ap, 		config->wifi_ap_switch,  NULL, 0, NULL );
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_wifi_ap_dhcps, 	config->wifi_ap_dhcps_switch, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_wifi_sta, 		config->wifi_sta_switch, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_wifi_sta_dhcpc, config->dhcp_client_switch, NULL, 0, NULL);

	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_dhcps_range_low, config->dhcps_range_low, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_dhcps_range_high, config->dhcps_range_high, NULL, 0, NULL);

	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_sta_ipv4, config->sta_ipv4, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_sta_ipv4netmask, config->sta_ipv4netmask, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_sta_ipv4gw, config->sta_ipv4gw, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_sta_ipv4ns1, config->sta_ipv4ns1, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_sta_ipv4ns2, config->sta_ipv4ns2, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_sta_ntpv4, config->ntpv4, NULL, 0, NULL);
	
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_ap_ipv4, config->ap_ipv4, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_ap_ipv4netmask, config->ap_ipv4netmask, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_ap_ipv4gw, config->ap_ipv4gw, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_ap_ipv4ns1, config->ap_ipv4ns1, NULL, 0, NULL);
	cfTagEncoder ( CP_INT32, str->msg, (uint32_t *)&i, cftag_ap_ipv4ns2, config->ap_ipv4ns2, NULL, 0, NULL);

	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_sta_ipv6, 			0, (unsigned char *)&config->sta_ipv6, 		16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_sta_ipv6netmask, 	0, (unsigned char *)&config->sta_ipv6netmask, 16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_sta_ipv6gw, 		0, (unsigned char *)&config->sta_ipv6gw, 		16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_sta_ipv6ns1, 		0, (unsigned char *)&config->sta_ipv6ns1, 	16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_sta_ipv6ns2, 		0, (unsigned char *)&config->sta_ipv6ns2, 	16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_sta_ntpv6, 			0, (unsigned char *)&config->ntpv6, 			16,  NULL);
	
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_ap_ipv6, 			0, (unsigned char *)&config->ap_ipv6, 		16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_ap_ipv6netmask, 	0, (unsigned char *)&config->ap_ipv6netmask, 	16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_ap_ipv6gw, 			0, (unsigned char *)&config->ap_ipv6gw, 		16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_ap_ipv6ns1, 		0, (unsigned char *)&config->ap_ipv6ns1, 		16,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_ap_ipv6ns2, 		0, (unsigned char *)&config->ap_ipv6ns2, 		16,  NULL);

	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_hostname, 			0, (unsigned char *)&config->hostname, 		32,  NULL);

	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_wifi_ap_ssid, 		0, (unsigned char *)&config->wifi_ap_ssid, 	32,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_wifi_ap_passwd, 	0, (unsigned char *)&config->wifi_ap_passwd, 	64,  NULL);

	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_wifi_sta_ssid, 		0, (unsigned char *)&config->wifi_sta_ssid, 	32,  NULL);
	cfTagEncoder ( CP_DATA8, str->msg, (uint32_t *)&i, cftag_wifi_sta_passwd, 	0, (unsigned char *)&config->wifi_sta_passwd, 	64,  NULL);



}
