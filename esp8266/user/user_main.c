#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include "driver/uuid.h"
#include "ntp.h"
#include <salsa20.h>
#include <poly1305-donna.h>

time_t ntp_unix_timestamp = 0;
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop();
LOCAL struct espconn conn;

LOCAL os_timer_t boottimer;
LOCAL os_timer_t poketimer;


LOCAL void 
ICACHE_FLASH_ATTR
arc4random_buf(unsigned char *b, int len ) 
{
	int i;
	for (i=0; i<len; i++ ) {
		b[i] = (unsigned char)os_random();	
	}
}


LOCAL void ICACHE_FLASH_ATTR
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



LOCAL void ICACHE_FLASH_ATTR print_bin2hex(unsigned char * x, int len) {
	int i;
	for (i=0; i<len; i++) {
		os_printf ( "%02x", x[i] );
		if ( (i>0) && ( (i+1)%4 == 0 ) ) { printf (" "); }
	}
	os_printf ("\n");
}



LOCAL void ICACHE_FLASH_ATTR
doCrypt(void)
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

	arc4random_buf((unsigned char *)&mysecret, 32);
	arc4random_buf((unsigned char *)&theirsecret, 32);
	uint8_t nonce[8]; 

	arc4random_buf((unsigned char *)&nonce, 8);
	

	unsigned char shared[32];


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
	
	curve25519_donna((unsigned char *)&shared, (unsigned char *)&mypublic, (unsigned char *)&theirpublic);
	printf (" Shared Key : " );
	print_bin2hex((unsigned char *)&shared, 32);
	
	printf (" Nonce : " ) ;
	print_bin2hex((unsigned char *)&nonce, 8);

	unsigned char mac[16];
	poly1305_auth(mac, _lyrics, L, shared);

	printf("MAC : ");		
	print_bin2hex((unsigned char *)&mac, 16);
	
	unsigned char *data=(unsigned char *)os_malloc(L+16,sizeof(unsigned char));
	memcpy(data, &mac, 16);
	memcpy(data+16, _lyrics, L);
	
	printf("Plain Text : %d \n", L);		
	hexprint( data, L+16);
	
	
	printf ( "\n\n");
	if (s20_crypt((uint8_t*)&shared, S20_KEYLEN_256, nonce, 0, data, L+16)) {
		printf("Error: encryption failed\n");
	} else {
		printf("Successful encryption \n");
		hexprint( data, L+16);
		printf(" \n");
	}

	unsigned char * data2=(unsigned char *)os_malloc(L+16,sizeof(unsigned char));
	memcpy(data2, data, L+16);
	
	if (s20_crypt((uint8_t*)&shared, S20_KEYLEN_256, nonce, 0, data2, L+16)) {
		printf("Error: decryption failed\n");
	} else {
		printf("Successful decryption \n");
		hexprint( data2, L+16);		
		printf(" \n");
	}

	unsigned char * plaintext=(unsigned char *)os_malloc(L,sizeof(unsigned char));
	
	unsigned char mac2[16];
//	memcpy(&mac2, data, 16);
	memcpy(plaintext, data2+16, L);
//	plaintext[0] = 'D';
//	plaintext[1] = 'P';
//	plaintext[2] = 'D';
	
	printf("Plain Text : %d \n", L);		
	hexprint( plaintext, L);
	
	poly1305_auth(mac2, plaintext, L, shared);
	printf("MAC : ");		
	print_bin2hex((unsigned char *)&mac2, 16);	

	
	int r = poly1305_verify(mac2, mac);
	
	
	printf("MAC Verify (%d) (%s)\n", r, r ? "correct" : "incorrect");

	
	
	os_free(plaintext);
	
	os_free(data2);
	os_free(data);

	//  curve25519_donna(shared, mysecret, theirpublic);

  
}


LOCAL void ICACHE_FLASH_ATTR
udp_callback(void *arg, char *data, unsigned short length)
{
    char DeviceBuffer[1400] = {0};
	char *INVAL = "Unknown command.\n\0";
	char *OFF = "Turning relay off.\n\0";
	char *ON = "Turning relay on.\n\0";
	char *UUID = "Got UUID\n\0";
	unsigned short l;
	struct uuid uuid;
	uint32 t;
	t = system_get_time();
	os_printf("%10u ==> GOT A UDP PACKET!\n\r", t );

    if (data == NULL) {
        return;
    }
	t = system_get_time();
	
	os_sprintf(DeviceBuffer, "%s", data );
//	l = os_strlen(DeviceBuffer);
/*
	if ( ( DeviceBuffer[length-1] == '\r' ) || ( DeviceBuffer[length-1] == '\n' ) ) {
		DeviceBuffer[length-1] = '\0';
		l--;
	}
*/

	os_printf("%10u ==> %s, length %u %u\n\r", t, DeviceBuffer, length, l );

	if ( 
			( data[0] == 'o' ) && 
			( data[1] == 'f' ) && 
			( data[1] == 'f' ) 
	) {
		gpio_output_set(BIT2, 0, BIT2, 0);
		espconn_sent(&conn, OFF, os_strlen(OFF));
	} else if ( 
			( data[0] == 'o' ) && 
			( data[1] == 'n' )
	) {
		gpio_output_set(0, BIT2, BIT2, 0);
		espconn_sent(&conn, ON, os_strlen(ON));
	} else if ( data[0] == 0xFF ) {
	    be_uuid_dec(data+1, &uuid);
	    os_printf("================== UDP uuid  ==========================\n ======> ");
	    printf_uuid(&(uuid));
    	os_printf ("\n");    		
    	print_bin2hex((unsigned char *)data, length);
	    os_printf("\n ================== UDP uuid  ==========================\n ");
		espconn_sent(&conn, UUID, os_strlen(UUID));
	} else {
		espconn_sent(&conn, INVAL, os_strlen(INVAL));
	}
	
	return;
	
}


static void ICACHE_FLASH_ATTR
loop()
{
	uint32 t,e,z;
	uint8 status;
	struct uuid uuid[1];
	t = system_get_time();
	status = wifi_station_get_connect_status();
    os_printf("%10u wifi connect status -  %d\n\r", t/1000000, status);
    os_printf("%10u Unix Timestamp \n\r", ntp_unix_timestamp);
    ntp_unix_timestamp++;
    
	kern_uuidgen(&(uuid[0]), 1);
    os_printf("uuid : ");
    printf_uuid(&(uuid[0]));
    os_printf ("\n\n");

    os_timer_disarm(&poketimer);
	t = system_get_time();
    doCrypt();
    e = system_get_time();
    z = e - t;
    printf ( " ===> Crypt Timer: %10d %10d %10d \n", t, e, z/1000);
   	os_timer_arm(&poketimer, 1000, 1);
    
    
}

static void ICACHE_FLASH_ATTR
startup()
{
	uint32 t;
	uint8 status;
	char buffer[128] = {0};
    struct ip_info ipconfig;
    char hwaddr[6];
	t = system_get_time();
	status = wifi_station_get_connect_status();
    os_printf("%10u wifi connect status -  %d\n\r", t/100000, status);

	if ( status == STATION_GOT_IP ) {
		os_timer_disarm(&boottimer);
		ntp_get_time();
        wifi_get_ip_info(STATION_IF, &ipconfig);
        wifi_get_macaddr(STATION_IF, hwaddr);
        os_sprintf(buffer, MACSTR " " IPSTR, 
                   MAC2STR(hwaddr), IP2STR(&ipconfig.ip));
		os_printf("%s\n\r", buffer);

    	conn.type = ESPCONN_UDP;
    	conn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    	conn.proto.udp->local_port = 1288;
    	espconn_regist_recvcb(&conn, udp_callback);
    	espconn_create(&conn);

	    os_timer_disarm(&poketimer);
    	os_timer_setfn(&poketimer, (os_timer_func_t *)loop, NULL);
    	os_timer_arm(&poketimer, 1000, 1);
	}
}


//Init function 
void ICACHE_FLASH_ATTR
user_init()
{
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config stationConf;

    uart_init(BIT_RATE_115200,BIT_RATE_115200);
    // Initialize the GPIO subsystem.
    gpio_init();
    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	gpio_output_set(BIT2, 0, BIT2, 0);

	os_delay_us(65534);
	os_delay_us(65534);
	os_delay_us(65534);
	os_delay_us(65534);
	os_delay_us(65534);
	os_delay_us(65534);
	os_delay_us(65534);

    os_printf("\n\n Booting ... \n\r");
	os_delay_us(65534);
    os_printf("  ______  _____ _____   ___ ___   __    __   \n\r");
	os_delay_us(65534);
    os_printf(" |  ____|/ ____|  __ \\ / _ \\__ \\ / /   / /   \n\r");
	os_delay_us(65534);
    os_printf(" | |__  | (___ | |__) | (_) | ) / /_  / /_   \n\r");
	os_delay_us(65534);
    os_printf(" |  __|  \\___ \\|  ___/ > _ < / / '_ \\| '_ \\  \n\r");
	os_delay_us(65534);
    os_printf(" | |____ ____) | |    | (_) / /| (_) | (_) | \n\r");
	os_delay_us(65534);
    os_printf(" |______|_____/|_|     \\___/____\\___/ \\___/  \n\r");
	os_delay_us(65534);
    os_printf("                                             \n\r");
	os_delay_us(65534);
    os_printf("                                             \n\r");


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
