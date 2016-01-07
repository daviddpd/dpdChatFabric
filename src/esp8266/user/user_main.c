#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "driver/gpio16.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include <salsa20.h>
#include <poly1305-donna.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "esp8266.h"
#include "util.h"
#include "esp-cf-config.h"
#include "esp-cf-wifi.h"
#include "uuid_wrapper.h"
#include "ntp.h"

//extern enum deviceModes currentMode;

extern time_t ntp_unix_timestamp;
extern int ntp_status;
//extern char macAddr[];
extern hostmeta_t hostMeta;


int bootstatus = 0;  // 1 - network up, 2-ready
// os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop();
void CP_ICACHE_FLASH_ATTR user_init_stage2();
//uint32_t ninc = 0;

int uart0enabled = -1;
int shiftCounter = -1;
uint8 shiftBits0[8];
uint8 shiftBits1[8];
//char ssid[32] = SSID;
//char password[64] = SSID_PASSWORD;

int SR_DATA = 5;
int SR_SRCLK1 = 12;
int SR_SRCLK2 = 14;
int SR_SRCLK;
int SR_RCLK = 4;

uint32_t controls[16];

LOCAL os_timer_t boottimer;
LOCAL os_timer_t poketimer;

LOCAL os_timer_t buttonDebounce;
LOCAL os_timer_t shiftReg;
LOCAL os_timer_t statusReg;



enum button {	
	BUTTON_UNDEFINED,
	BUTTON_MENU,
	BUTTON_SELECT,
};


//void userWifiInit();
void chatFabricInit();

void userGPIOInit();
void statusLoop();
void udp_callback(void *arg, char *data, unsigned short length);
void shiftReg0();
void shiftReg1();
void changeMode(enum deviceModes m);
enum deviceModes menuItem = MODE_MENU_NONE;

void CP_ICACHE_FLASH_ATTR
doButtonFunction(enum button b) 
{

	if ( b == BUTTON_MENU ) 
	{
		menuItem++;
		if (menuItem == MODE_MENU_END ) {
			menuItem = MODE_MENU_IN;
		}
		os_printf ( " ==> Menu : %d \n", menuItem );
		changeMode(menuItem);
	}
	
	if ( b == BUTTON_SELECT && menuItem != MODE_MENU_NONE )  
	{
		os_printf ( " ==> Selecting : %d \n", menuItem );

		switch(menuItem) {
			case MODE_MENU_APMODE:
			break;
			case MODE_MENU_STAMODE:			
			break;
			case MODE_MENU_FACTORYRESET:
				shiftReg0();
				shiftReg1();

				currentMode = MODE_BOOTING;
				changeMode(MODE_BOOTING);
	
				os_timer_disarm(&statusReg);
				os_timer_setfn(&statusReg, (os_timer_func_t *)statusLoop, NULL);
				os_timer_arm(&statusReg, 300, 1);
	
				// uart_init(BIT_RATE_115200,BIT_RATE_115200);
				// Initialize the GPIO subsystem.
				chatFabricInit();
				//userWifiInit();
			break;
			
			case MODE_MENU_UNPAIRALL:
				cfPairInit(&pair[0]);
				currentMode = MODE_STA_UNPAIRED;
				changeMode(MODE_STA_UNPAIRED);				
				os_printf ( " ==> Unpaired : %d \n", menuItem );
			break;
		}
		

		menuItem = MODE_MENU_NONE;
		changeMode(currentMode);

	}
	
}


void CP_ICACHE_FLASH_ATTR
changeMode(enum deviceModes m) {
//	os_printf ( " ==> changeMode %d\n", m );

	int i = 0;
	for ( i=7; i>=0; i-- ) {
		shiftBits1[i] = 0;
	}
	
	switch (m) {
		case MODE_UNCONFIGURED:
			shiftBits1[0] = 0x01;
		break;
		case MODE_BOOTING:
			shiftBits1[0] = 0x03;
			shiftBits1[1] = 0x02;		
		break;
		
		case MODE_AP_UNPAIRED:
			shiftBits1[5] = 1;
			shiftBits1[4] = 0x03;			
		break;
		
		case MODE_AP_PAIRED:
			shiftBits1[5] = 1;
			shiftBits1[4] = 1;
		break;

		case MODE_STA_NOWIFI:
			shiftBits1[3] = 1;
			shiftBits1[2] = 0;
			shiftBits1[0] = 0x03;
			shiftBits1[1] = 0x03;
		break;			
		case MODE_STA_UNPAIRED:
			shiftBits1[3] = 1;
			shiftBits1[2] = 0x03;			
		break;
		case MODE_STA_PAIRED:
			shiftBits1[3] = 1;
			shiftBits1[2] = 1;
		break;
		
		case MODE_MENU_IN:
		case MODE_MENU_APMODE:
		case MODE_MENU_STAMODE:
		case MODE_MENU_FACTORYRESET:
		case MODE_MENU_UNPAIRALL:
			shiftBits1[7] = 0x03;
			shiftBits1[6] = 0x02;		
			if ( m == MODE_MENU_APMODE ) 		{ shiftBits1[5] = 1; }
			if ( m == MODE_MENU_STAMODE ) 		{ shiftBits1[3] = 1; }
			if ( m == MODE_MENU_FACTORYRESET )	{ shiftBits1[1] = 1; shiftBits1[0] = 1; }
			if ( m == MODE_MENU_UNPAIRALL ) 	{ shiftBits1[5] = 1; shiftBits1[3] = 1; }
		break;
		default:
			os_printf ( " ==> changeMode %d - mode not found, case default.\n", m );
		break;
	}
/*
	os_printf ( " ==> shiftBits: " );
	for ( i=7; i>=0; i-- ) {
		os_printf ( "%02x ", shiftBits1[i] );
	}
	os_printf ( "\n");	
*/

}



void CP_ICACHE_FLASH_ATTR
shiftReg1() {
	SR_SRCLK = SR_SRCLK2; 
	GPIO_OUTPUT_SET(SR_SRCLK, 0);
	int i;
	for ( i=7; i>=0; i-- ) {
		
		uint8 value = shiftBits1[i] & 0x01;
		uint8 isBlink = shiftBits1[i] & 0x02;
		
		uint8 new_shiftBits = 0;
		if ( isBlink ) {
			new_shiftBits = shiftBits1[i] ^ 0x01;
			shiftBits1[i] = new_shiftBits;
		} 
		
		if ( value ) {
			GPIO_OUTPUT_SET(SR_DATA, 1);
		} else {
			GPIO_OUTPUT_SET(SR_DATA, 0);
		}		

		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 1);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		
	}

	os_delay_us(10);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		
	os_delay_us(10);
	GPIO_OUTPUT_SET(SR_RCLK, 1);
	os_delay_us(10);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		

//	os_printf("\n");

}


//LOCAL os_timer_t buttonDebounce;
void CP_ICACHE_FLASH_ATTR
shiftReg0() {

	int i;

	if (shiftCounter == -1 ) {
		for ( i=7; i>=0; i-- ) {
			shiftBits0[i] = 0;
			shiftBits1[i] = 0;
		}
		shiftCounter =0;
	} 



	SR_SRCLK = SR_SRCLK1; 
	GPIO_OUTPUT_SET(SR_SRCLK, 0);
	for ( i=7; i>=0; i-- ) {
		
		if ( shiftBits0[i] ) {
			GPIO_OUTPUT_SET(SR_DATA, 1);
		} else {
			GPIO_OUTPUT_SET(SR_DATA, 0);
		}		
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 1);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		
	}

	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		
	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 1);
	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		

}

void CP_ICACHE_FLASH_ATTR 
gpioInitFromConfig(chatFabricConfig *config) 
{

	int i;
	for (i=0; i<config->numOfControllers; i++) 
	{
		if (config->controlers[i].gpio == 16) {
			gpio16_output_set(config->controlers[i].value);
		} else {
			GPIO_OUTPUT_SET(config->controlers[i].gpio, config->controlers[i].value);
			printf ( "=== %10s: %4d %24s %4d %4d \n", "Setting", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].gpio );
		}		
	}			

}

void CP_ICACHE_FLASH_ATTR 
deviceCallBack(chatFabricConfig *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd) 
{

	int i=0, x=0;

	unsigned char *tmp;


	for (i=0; i<config->numOfControllers; i++) 
	{
		if ( config->controlers[i].control == cp->action_control ) 
		{
			if ( i == 0 ){
			    config->controlers[0].value = ntp_unix_timestamp;
			}
	
			if (cp->action == ACTION_GET ) 
			{

				reply->action = ACTION_READ;
				reply->action_control = cp->action_control;
				reply->action_type = config->controlers[i].type;
				reply->action_value = config->controlers[i].value;
				reply->action_length = 0;
		
		
			} else if (cp->action == ACTION_SET ) 
			{
				config->controlers[i].value = cp->action_value;

				reply->action = ACTION_READ;
				reply->action_control = cp->action_control;
				reply->action_type = config->controlers[i].type;
				reply->action_value = config->controlers[i].value;
				reply->action_length = 0;	

				if  ( 	config->controlers[i].type == ACTION_TYPE_BOOLEAN ) {
					if ( cp->action_value ) {
						if (config->controlers[i].gpio == 16) {
							gpio16_output_set(1);
						} else {
							GPIO_OUTPUT_SET(config->controlers[i].gpio, 1);
							printf ( "=== %10s: %4d %24s %4d %4d \n", "Setting", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].gpio );
						}
					} else {
						if (config->controlers[i].gpio == 16) {
							gpio16_output_set(0);
						} else {
							GPIO_OUTPUT_SET(config->controlers[i].gpio, 0);
							printf ( "=== %10s: %4d %24s %4d %4d \n", "Setting", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].gpio );
						}
					}
				}
				if  ( 	config->controlers[i].type == ACTION_TYPE_DIMMER ) {
					
					for (x=1; x<=8; x++) {					
						if ( cp->action_value == 0  ) {
							shiftBits0[x-1] = 0;
						} else if ( x <= cp->action_value ) {
							shiftBits0[x-1] = 1;
						} else {
							shiftBits0[x-1] = 0;
						
						}
					}
					shiftReg0();
				}				
				
				
			}			
		}

		printf ( "=== %10s: %4d %24s %4d %4d \n", "Control", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].gpio );

	}


}

void CP_ICACHE_FLASH_ATTR
udp_callback(void *arg, char *data, unsigned short length)
{
    msgbuffer payloadMsg;
	unsigned short l;
	struct uuid uuid;
	uint32 t;
	enum chatFabricErrors e;
//	t = system_get_time();
	printf ( "Got network Packet.\n" );
//    os_printf("%12u %12u  GOT A UDP PACKET\n\r", t/100000, ntp_unix_timestamp);

    if (data == NULL) {
        return;
    }
    
    c.conn = arg;
    payloadMsg.length = 0;
    payloadMsg.msg = 0;
    mbuff.length = (int)length;
    mbuff.msg = data;

	printf ( "chatFabric_device call.\n" );
    e = chatFabric_device(&c, &pair[0], &config,  &payloadMsg);
	printf( "chatFabric_device return.\n" );

    if  ( ( ERROR_OK == e ) && ( payloadMsg.length > 0) ) {
/*
		if ( payloadMsg.length == 3 ) {
			if ( ( payloadMsg.msg[0] == 'o' ) &&
				 ( payloadMsg.msg[1] == 'f' ) &&
				 ( payloadMsg.msg[1] == 'f' )		
			) {
				
				//gpio_output_set(BIT2, 0, BIT2, 0);
				GPIO_OUTPUT_SET(5, 0);
			}
		}
		
		if ( payloadMsg.length == 2 ) {
			if ( ( payloadMsg.msg[0] == 'o' ) &&
				 ( payloadMsg.msg[1] == 'n' )
			) {
				//gpio_output_set(0, BIT2, BIT2, 0);
				GPIO_OUTPUT_SET(5, 1);
			}
		}

		char * str = (char * )calloc(payloadMsg.length+1,sizeof (char) );
		str[payloadMsg.length] = '\0';		
		memcpy(str, payloadMsg.msg, payloadMsg.length );
		os_printf("%12u heap: %12d heapDiff: %12d :: Payload %32s\n\r", ntp_unix_timestamp, heap, heapLast-heap, str);
		free(str);
*/		
		free(payloadMsg.msg);
	}
	printf( "udp_callback return.\n" );
	return;
}


void CP_ICACHE_FLASH_ATTR
tcp_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

    espconn_regist_recvcb(pesp_conn, udp_callback);
    
//    espconn_regist_reconcb(pesp_conn, webserver_recon);
//    espconn_regist_disconcb(pesp_conn, webserver_discon);
}


static void CP_ICACHE_FLASH_ATTR
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
		
//	os_printf(".");
//	os_printf(" Time: %8d \n", ntp_unix_timestamp );
//	bootstatus++;

}

void CP_ICACHE_FLASH_ATTR
statusLoop() {
	uuid_cp u1;
	
	if ( pair[0].hasPublicKey && ( menuItem == MODE_MENU_NONE ) ) {
		currentMode = MODE_STA_PAIRED;
		changeMode(MODE_STA_PAIRED);
	} 

	shiftReg1();
/*
	uuidCreate( &(u1));	
	os_printf("uuid 1: ");
	printf_uuid(&(u1));
	os_printf("\n");
*/
}

/*
static void CP_ICACHE_FLASH_ATTR
startup_station()
{
	uint32 t;
	uint32_t status;
	char buffer[128] = {0};
	char buffer2[16] = {0};
    struct ip_info ipconfig;
    char hwaddr[6];
	t = system_get_time();
	wifiStatus = wifi_station_get_connect_status();
	heapLast = heap;
	heap = system_get_free_heap_size();	
    os_printf("[%6d] %12u %12u wifi connect status -  %d/%d\n\r", heapLast - heap, t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);

	if ( ( wifiStatus == STATION_GOT_IP ) &&  ( bootstatus == 0 ) ) {
		os_timer_disarm(&boottimer);
        wifi_get_ip_info(STATION_IF, &ipconfig);
        wifi_get_macaddr(STATION_IF, hwaddr);
        os_sprintf(buffer, MACSTR " " IPSTR, 
                   MAC2STR(hwaddr), IP2STR(&ipconfig.ip));
		os_printf(" ==> GOT IP: %s\n\r", buffer);

		bootstatus = 1; // network up
		
		currentMode = MODE_STA_UNPAIRED;
		changeMode(MODE_STA_UNPAIRED);


		os_printf(" ==> Call NTP Time \n\r", buffer);
		ntp_get_time();
		// ntp_unix_timestamp = 1437438241;
		os_timer_disarm(&boottimer);
	    os_timer_setfn(&boottimer, (os_timer_func_t *)startup_station, NULL);
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
		
		b.length = -1;

		cfPairInit(&pair[0]);

		
	
	    os_printf("%12u %12u IDS -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);

		if ( flashConfig[2048] == cftag_header ) {
			os_printf("reading pair config\n");
			cfPairRead(&config, (chatFabricPairing *)&(pair[0]) );
		}
		if ( config.hasPairs ) {
			currentMode = MODE_STA_PAIRED;
			changeMode(MODE_STA_PAIRED);
		} else {
			currentMode = MODE_STA_UNPAIRED;
			changeMode(MODE_STA_UNPAIRED);
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

//		CHATFABRIC_DEBUG_B2H(1, "publicKey", (unsigned char *)&config.publickey, crypto_box_PUBLICKEYBYTES );
//		CHATFABRIC_DEBUG_B2H(1, "privateKey", (unsigned char *)&config.privatekey, crypto_box_SECRETKEYBYTES );

		bootstatus = 2; // timeset 
	    os_printf("%12u %12u BootStatus -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);

		c.tcpconn.type = ESPCONN_TCP;
		c.tcpconn.state = ESPCONN_NONE;
		c.tcpconn.proto.tcp = &c.esptcp;
		c.tcpconn.proto.tcp->local_port = config.port;
	    os_printf("%12u %12u tcp_listen -  %d/%d Line:%d \n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus, __LINE__);
		espconn_regist_connectcb(&c.tcpconn, tcp_listen);
	    os_printf("%12u %12u tcp_accpet -  %d/%d Line:%d \n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus, __LINE__);
		espconn_accept(&c.tcpconn);

    	c.udpconn.type = ESPCONN_UDP;
	    os_printf("%12u %12u zalloc -  %d/%d Line:%d \n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus, __LINE__);
    	c.udpconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    	c.udpconn.proto.udp->local_port = config.port;
	    os_printf("%12u %12u udp callback -  %d/%d Line:%d \n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus, __LINE__);
    	espconn_regist_recvcb(&c.udpconn, udp_callback);
	    os_printf("%12u %12u udp conn -  %d/%d Line:%d \n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus, __LINE__);
    	espconn_create(&c.udpconn);

	    os_printf("%12u %12u mdns -  %d/%d Line:%d \n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus, __LINE__);
		espCfMdns();
	    os_printf("%12u %12u configwrite -  %d/%d Line:%d \n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus, __LINE__);
		cfConfigWrite(&config);

	    os_printf("%12u %12u UDP setup complete -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);
	} 
	
}

*/

void CP_ICACHE_FLASH_ATTR
doButton(uint8 gpio_pin)
{
	uint8 i = gpio_pin;	
	os_printf ( " ==> Starting doButton %d \n", i );		
	if ( currentMode == MODE_BOOTING ) {
		os_timer_disarm(&statusReg);	
		switch(i) {
			case 0:
				os_printf ( " ==> Pin %d pressed - Booting stage 2\n", i );
				user_init_stage2();
			break;
		}	
	} else {

		switch(i) {
			case 0:
				os_printf ( " ==> Pin %d pressed - Menu\n", i );
				doButtonFunction(BUTTON_MENU);
			break;
			case 2:
				os_printf ( " ==> Pin %d pressed - Select\n", i );
				doButtonFunction(BUTTON_SELECT);
			break;
			default:
				os_printf ( " ==> Pin %d pressed \n", i );			
			break;
		}

	}
	gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_NEGEDGE);
}

void CP_ICACHE_FLASH_ATTR
buttonPress(void *n)
{
	uint8 i = 0;
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	os_printf ( " ==> buttonPress interrupt \n");		

	for (i=0; i<16; i++) {
		if (gpio_status & BIT(i) ) {
			//disable interrupt
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_DISABLE);

			//clear interrupt status
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(i));

		    os_timer_disarm(&buttonDebounce);
		    os_timer_setfn(&buttonDebounce, (os_timer_func_t *)doButton, i);
		    os_timer_arm(&buttonDebounce, 200, 0);
		    
		    
		}		
	}

}

void CP_ICACHE_FLASH_ATTR
userGPIOInit()
{

    gpio_init();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);

	gpio16_output_conf();
	gpio16_output_set(0);
    
	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH(buttonPress, NULL);
	GPIO_DIS_OUTPUT(0); // set for input
	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(0); // set for input
//	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(3); // set for input

//    gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_NEGEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_NEGEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_NEGEDGE);    
//    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_NEGEDGE);    
//    gpio_pin_intr_state_set(GPIO_ID_PIN(3), GPIO_PIN_INTR_NEGEDGE);    

//  gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_NEGEDGE);
    ETS_GPIO_INTR_ENABLE();

}


void CP_ICACHE_FLASH_ATTR
chatFabricInit()
{
    int i=0;
	bzero(&c,sizeof(c));
	bzero(&config,sizeof(config));	
	bzero(&b,sizeof(b));
	bzero(&hostMeta, sizeof(hostMeta));
	hostMeta.status = 0;

	for (i=0; i<16; i++) {
		cpStatus[i] = -1;
	}

	for (i=0; i<4096; i++ ){
		flashConfig[i] = '\0';
	}

	heap =0;
	heapLast = 0;
	heapLast = heap;
	heap = system_get_free_heap_size();
		

}


void CP_ICACHE_FLASH_ATTR
user_init_stage2() 
{
	chatFabricInit();

	createHostMeta();
	espCfConfigInit();

	cfPairInit(&pair[0]);
	if ( flashConfig[2048] == cftag_header ) {
		os_printf("reading pair config\n");
		cfPairRead(&config, (chatFabricPairing *)&(pair[0]) );
	}
	if ( config.hasPairs ) {
		currentMode = MODE_STA_PAIRED;
		changeMode(MODE_STA_PAIRED);
	} else {
		currentMode = MODE_STA_UNPAIRED;
		changeMode(MODE_STA_UNPAIRED);
	}

	char buf[38];
	snprintf_uuid(buf, sizeof(buf), &config.uuid.u0);
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	snprintf_uuid(buf, sizeof(buf), &config.uuid.u1);
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	espWiFiInit();

	shiftReg0();
	shiftReg1();

	cfConfigWrite(&config);
	_GLOBAL_DEBUG = config.debug;
	gpioInitFromConfig(&config);

	os_timer_disarm(&statusReg);
	os_timer_setfn(&statusReg, (os_timer_func_t *)statusLoop, NULL);
	os_timer_arm(&statusReg, 300, 1);

	
}
void CP_ICACHE_FLASH_ATTR
bootwait() {
	CHATFABRIC_PRINT ("Press Button to Boot\n");
}


//Init function 
void CP_ICACHE_FLASH_ATTR
user_init()
{
	int i;
	uart_init(BIT_RATE_115200,BIT_RATE_115200);

	currentMode = MODE_BOOTING;
	changeMode(MODE_BOOTING);
	uart0enabled = 1;
	userGPIOInit();
	user_init_stage2();

//	os_timer_disarm(&statusReg);
//	os_timer_setfn(&statusReg, (os_timer_func_t *)bootwait, NULL);
//	os_timer_arm(&statusReg, 250, 1);

}