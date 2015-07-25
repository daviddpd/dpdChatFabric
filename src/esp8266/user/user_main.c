#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include "uuid.h"
#include "ntp.h"
#include <salsa20.h>
#include <poly1305-donna.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "esp8266.h"

time_t ntp_unix_timestamp = 0;
int ntp_status = -1;
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
int bootstatus = 0;  // 1 - network up, 2-ready
// os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop();
//uint32_t ninc = 0;


LOCAL os_timer_t boottimer;
LOCAL os_timer_t poketimer;
/*
LOCAL void ICACHE_FLASH_ATTR
nonceInc()
{
	memcpy ( &ninc, (unsigned char *)&(pair[0].nullnonce[4]), 4);
	uint32_t ninc2 = be32dec((void *)&ninc);
	
	printf ( "A %8u ", ninc ); 
	print_bin2hex( (unsigned char *)&ninc, 4 );

	printf ( "A %8u ", ninc2 ); 
	print_bin2hex( (unsigned char *)&ninc2, 4 );

	ninc2++;
	ninc = 0;
	ninc = be32dec((void *)&ninc2);

	printf ( "C %8u ", ninc ); 
	print_bin2hex( (unsigned char *)&ninc, 4 );

	printf ( "C %8u ", ninc2 ); 
	print_bin2hex( (unsigned char *)&ninc2, 4 );
	
	memcpy ( &(pair[0].nullnonce[4]), &ninc, 4);
	

}
*/
LOCAL void ICACHE_FLASH_ATTR
udp_callback(void *arg, char *data, unsigned short length)
{
    msgbuffer payloadMsg;
	unsigned short l;
	struct uuid uuid;
	uint32 t;
//	t = system_get_time();
	CHATFABRIC_DEBUG(config.debug, "Got UDP Packet." );
//    os_printf("%12u %12u  GOT A UDP PACKET\n\r", t/100000, ntp_unix_timestamp);

    if (data == NULL) {
        return;
    }
    
    payloadMsg.length = 0;
    payloadMsg.msg = 0;
    mbuff.length = (int)length;
    mbuff.msg = data;

	CHATFABRIC_DEBUG(config.debug, "chatFabric_device call" );
    chatFabric_device(&c, &pair[0], &config,  &payloadMsg);
	CHATFABRIC_DEBUG(config.debug, "chatFabric_device return" );

    if  ( payloadMsg.length > 0 ) {


		if ( payloadMsg.length == 3 ) {
			if ( ( payloadMsg.msg[0] == 'o' ) &&
				 ( payloadMsg.msg[1] == 'f' ) &&
				 ( payloadMsg.msg[1] == 'f' )		
			) {
				gpio_output_set(BIT2, 0, BIT2, 0);
			}
		}
		
		if ( payloadMsg.length == 2 ) {
			if ( ( payloadMsg.msg[0] == 'o' ) &&
				 ( payloadMsg.msg[1] == 'n' )
			) {
				gpio_output_set(0, BIT2, BIT2, 0);
			}
		}

		char * str = (char * )calloc(payloadMsg.length+1,sizeof (char) );
		str[payloadMsg.length] = '\0';		
		memcpy(str, payloadMsg.msg, payloadMsg.length );
		os_printf("%12u heap: %12d heapDiff: %12d :: Payload %32s\n\r", ntp_unix_timestamp, heap, heapLast-heap, str);
		free(str);
		free(payloadMsg.msg);
	}
	CHATFABRIC_DEBUG(config.debug, "udp_callback return." );
	return;
}


static void ICACHE_FLASH_ATTR
loop()
{
	uint32 t,e,z;
	uint8 status;
	if (ntp_unix_timestamp > 0) {
	    ntp_unix_timestamp++;
	}
	t = system_get_time();
	heapLast = heap;
	heap = system_get_free_heap_size();
	
	wifiStatus = wifi_station_get_connect_status();
//	nonceInc(cp100, &config, &pair[0]);
//	printf (" Nonce : " );
//	print_bin2hex( (unsigned char *)&cp100->nonce,  crypto_secretbox_NONCEBYTES);

	os_printf("%12u %12u heap: %12d heapDiff: %12d  wifi/bootmode: %d/%d\n\r", t/100000, ntp_unix_timestamp, heap, heapLast-heap, wifiStatus, bootstatus);
}

static void ICACHE_FLASH_ATTR
startup()
{
	uint32 t;
	uint32_t status;
	char buffer[128] = {0};
    struct ip_info ipconfig;
    char hwaddr[6];
	t = system_get_time();
	wifiStatus = wifi_station_get_connect_status();
    os_printf("%12u %12u wifi connect status -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);

	if ( ( wifiStatus == STATION_GOT_IP ) &&  ( bootstatus == 0 ) ) {
		os_timer_disarm(&boottimer);
        wifi_get_ip_info(STATION_IF, &ipconfig);
        wifi_get_macaddr(STATION_IF, hwaddr);
        os_sprintf(buffer, MACSTR " " IPSTR, 
                   MAC2STR(hwaddr), IP2STR(&ipconfig.ip));
		os_printf("%s\n\r", buffer);

		bootstatus = 1; // network up
		ntp_get_time();
		// ntp_unix_timestamp = 1437438241;
		os_timer_disarm(&boottimer);
	    os_timer_setfn(&boottimer, (os_timer_func_t *)startup, NULL);
    	os_timer_arm(&boottimer, 250, 1);
		
	} 
	if ( (ntp_status == 2) && ( bootstatus == 1 ) ) {
		os_printf ("ntp timed out ... retry.\n");
		ntp_get_time();	
	}
	
	if ( (ntp_unix_timestamp > 0) && ( bootstatus == 1 ) ) {
		os_timer_disarm(&boottimer);

	    os_timer_disarm(&poketimer);
    	os_timer_setfn(&poketimer, (os_timer_func_t *)loop, NULL);
    	os_timer_arm(&poketimer, 1000, 1);


	    os_printf("%12u %12u Initializing Chat Fabric -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);

//		bzero(&c,sizeof(c));	
//		bzero(&pair[0],sizeof(pair[0]));	
//		bzero(&config,sizeof(config));	
//		bzero(&b,sizeof(b));

		pair[0].state = STATE_UNCONFIGURED;
		pair[0].hasPublicKey = 0;
	
		uuid_create_nil ( &(pair[0].uuid.u0), &status);
		uuid_create_nil ( &(pair[0].uuid.u1), &status);
		pair[0].hasPublicKey = 0;
		pair[0].hasNonce = 0;
		pair[0].serial = 0;
		
		b.length = -1;
		arc4random_buf((unsigned char *)&(pair[0].mynonce), crypto_secretbox_NONCEBYTES);
		bzero(&(pair[0].nullnonce), crypto_secretbox_NONCEBYTES);
		config.debug = 1;
		chatFabric_configParse(&config);
	    os_printf("%12u %12u IDS -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);

		if ( flashConfig[2048] == cftag_header ) {
			os_printf("reading pair config\n");
			chatFabric_pairConfig(&config, (chatFabricPairing *)&pair[0], 0 );
		}

	    os_printf("%12u %12u FlashConfig  %02x %02x %02x %02x %02x \n\r", t/100000, ntp_unix_timestamp, flashConfig[2048], flashConfig[2049], flashConfig[2050], flashConfig[2051], flashConfig[2052] );
	    
		os_printf("state : %s\n", stateLookup(pair[0].state) );
	    
		os_printf("uuid 0: ");
		printf_uuid(&(config.uuid.u0));
		os_printf ("\n\n");	    

		os_printf("uuid 1: ");
		printf_uuid(&(config.uuid.u1));
		os_printf ("\n\n");	    

	    os_printf("%12u %12u BootStatus print keys -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);
//		os_printf ( "%2s %24s: \n", ' ', "publicKey");
		print_bin2hex((unsigned char *)&config.publickey, crypto_box_PUBLICKEYBYTES);
// 		os_printf ( "%2s %24s: \n", ' ', "privateKey");
		print_bin2hex((unsigned char *)&config.privatekey, crypto_box_SECRETKEYBYTES);



		bootstatus = 2; // timeset 
	    os_printf("%12u %12u BootStatus -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);
    	c.conn.type = ESPCONN_UDP;
    	c.conn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    	c.conn.proto.udp->local_port = 1288;
    	espconn_regist_recvcb(&c.conn, udp_callback);
    	espconn_create(&c.conn);
	    os_printf("%12u %12u UDP setup complete -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);


	} 
	
}


//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;
    int i=0;

    uart_init(BIT_RATE_115200,BIT_RATE_115200);
    // Initialize the GPIO subsystem.
    gpio_init();
    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	gpio_output_set(BIT2, 0, BIT2, 0);

	bzero(&c,sizeof(c));	
//	bzero(&pair[0],sizeof(pair[0]));	
	bzero(&config,sizeof(config));	
	bzero(&b,sizeof(b));

	for (i=0; i<16; i++) {
		cpStatus[i] = -1;
	}

	for (i=0; i<4096; i++ ){
		flashConfig[i] = '\0';
	}

	heap =0;
	heapLast = 0;
    //Set station mode
    wifi_set_opmode( 0x1 );

    //Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    os_timer_disarm(&boottimer);
    os_timer_setfn(&boottimer, (os_timer_func_t *)startup, NULL);
    os_timer_arm(&boottimer, 250, 1);

    //Start os task
    // system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    // system_os_post(user_procTaskPrio, 0, 0 );
}
