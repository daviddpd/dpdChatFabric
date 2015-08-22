#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include "uuid_wrapper.h"
#include "ntp.h"
#include <salsa20.h>
#include <poly1305-donna.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "esp8266.h"
#include "util.h"

time_t ntp_unix_timestamp = 0;
int ntp_status = -1;
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
int bootstatus = 0;  // 1 - network up, 2-ready
// os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop();
//uint32_t ninc = 0;

int uart0enabled = -1;
int shiftCounter = -1;
unit8 shiftBits0[8];
unit8 shiftBits1[8];
char ssid[32] = SSID;
char password[64] = SSID_PASSWORD;
struct station_config stationConf;

int SR_DATA = 5;
int SR_SRCLK1 = 0;
int SR_SRCLK2 = 4;
int SR_SRCLK;
int SR_RCLK = 12;

uint32_t controls[16];

LOCAL os_timer_t boottimer;
LOCAL os_timer_t poketimer;

LOCAL os_timer_t buttonDebounce;
LOCAL os_timer_t shiftReg;
LOCAL os_timer_t statusReg;



void CP_ICACHE_FLASH_ATTR
shiftReg1() {
	SR_SRCLK = SR_SRCLK2; 
	GPIO_OUTPUT_SET(SR_SRCLK, 0);
	int i;
	for ( i=7; i>=0; i-- ) {
		
		unit8 value = shiftBits1[i]>>1;
		uint8 isBlink = ( shiftBits1[i] & 0x02 ) >> 2;
		shiftBits1[i] = shiftBits1[i] ^ value;
		
		
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

	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		
	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 1);
	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		


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
deviceCallBack(chatFabricConfig *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd) 
{

	int i=0, x=0;

	unsigned char *tmp;


	for (i=0; i<config->numOfControllers; i++) 
	{
		if ( config->controlers[i].control == cp->action_control ) 
		{
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

				if  ( cp->action_control == 0 ) {
					if ( cp->action_value ) {				
						GPIO_OUTPUT_SET(5, 1);
					} else {
						GPIO_OUTPUT_SET(5, 0);			
					}
				}
				
				if  ( cp->action_control == 1 ) {
					
					for (x=1; x<=8; x++) {
						//printf ( "=== %10s: %4d %24s %4d \n", "Control", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value );
					
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

		printf ( "=== %10s: %4d %24s %4d \n", "Control", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value );

	}


}

LOCAL void CP_ICACHE_FLASH_ATTR
udp_callback(void *arg, char *data, unsigned short length)
{
    msgbuffer payloadMsg;
	unsigned short l;
	struct uuid uuid;
	uint32 t;
	enum chatFabricErrors e;
//	t = system_get_time();
	CHATFABRIC_DEBUG(config.debug, "Got UDP Packet." );
//    os_printf("%12u %12u  GOT A UDP PACKET\n\r", t/100000, ntp_unix_timestamp);

    if (data == NULL) {
        return;
    }
    
    c.conn = arg;
    payloadMsg.length = 0;
    payloadMsg.msg = 0;
    mbuff.length = (int)length;
    mbuff.msg = data;

	CHATFABRIC_DEBUG(config.debug, "chatFabric_device call" );
    e = chatFabric_device(&c, &pair[0], &config,  &payloadMsg);
	CHATFABRIC_DEBUG(config.debug, "chatFabric_device return" );

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
	CHATFABRIC_DEBUG(config.debug, "udp_callback return." );
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
//	nonceInc(cp100, &config, &pair[0]);
//	printf (" Nonce : " );
//	util_print_bin2hex( (unsigned char *)&cp100->nonce,  crypto_secretbox_NONCEBYTES);

	//os_printf("%12u %12u heap: %12d heapDiff: %12d  wifi/bootmode: %d/%d\n\r", t/100000, ntp_unix_timestamp, heap, heapLast-heap, wifiStatus, bootstatus);
	
	os_printf(".");
	if ( shiftBits1[2] ) {	
		shiftBits1[2] = 0;
		shiftBits1[3] = 1;
	} else {
		shiftBits1[2] = 1;
		shiftBits1[3] = 0;	
	}
	shiftReg1();

}

void CP_ICACHE_FLASH_ATTR
statusLoop() {
	
	if ( shiftBits1[0] ) {	
		shiftBits1[0] = 0;
		shiftBits1[1] = 1;
	} else {
		shiftBits1[0] = 1;
		shiftBits1[1] = 0;	
	}
	shiftReg1();

}



static void CP_ICACHE_FLASH_ATTR
startup_station()
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

		os_printf ("Setting Up mDNS ... \n");
		struct mdns_info *info = (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
		info->ipAddr = ipconfig.ip.addr; //ESP8266 station IP

		info->server_name = "chatFabric";
		info->host_name = "esp8266";

        wifi_get_macaddr(STATION_IF, hwaddr);
        wifi_get_ip_info(STATION_IF, &ipconfig);
		
		info->server_port = 1288;
		info->txt_data[0] = "version = 0.1";
		info->txt_data[1] = "user1 = data1";
		info->txt_data[2] = "user2 = data2";
		espconn_mdns_init(info);


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

//		bzero(&c,sizeof(c));	
//		bzero(&pair[0],sizeof(pair[0]));	
//		bzero(&config,sizeof(config));	
//		bzero(&b,sizeof(b));


/*
		pair[0].state = STATE_UNCONFIGURED;
		pair[0].hasPublicKey = 0;
	
		uuid_create_nil ( &(pair[0].uuid.u0), &status);
		uuid_create_nil ( &(pair[0].uuid.u1), &status);
		pair[0].hasPublicKey = 0;
		pair[0].hasNonce = 0;
		pair[0].serial = 0;
		arc4random_buf((unsigned char *)&(pair[0].mynonce), crypto_secretbox_NONCEBYTES);
		bzero(&(pair[0].nullnonce), crypto_secretbox_NONCEBYTES);
*/
		chatFabric_pair_init(&pair[0]);

		config.pairfile = "1";
		
		b.length = -1;
		config.debug = 0;
		chatFabric_configParse(&config);
		config.callback = (void*)&deviceCallBack;

	config.numOfControllers = 2;
	config.controlers = (cfControl*)malloc(config.numOfControllers * sizeof(cfControl));
		
	int i =	0;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].label = "switch1";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;

/*
	i =	1;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].label = "switch2";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
*/
	
	i =1;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_DIMMER;
	config.controlers[i].value = 0;
	config.controlers[i].label = "Dimmer0";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 8;
			
		
	    os_printf("%12u %12u IDS -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);

		if ( flashConfig[2048] == cftag_header ) {
			os_printf("reading pair config\n");
			chatFabric_pairConfig(&config, (chatFabricPairing *)&(pair[0]), 0 );
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
		util_print_bin2hex((unsigned char *)&config.publickey, crypto_box_PUBLICKEYBYTES);
// 		os_printf ( "%2s %24s: \n", ' ', "privateKey");
		util_print_bin2hex((unsigned char *)&config.privatekey, crypto_box_SECRETKEYBYTES);



		bootstatus = 2; // timeset 
	    os_printf("%12u %12u BootStatus -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);


		c.tcpconn.type = ESPCONN_TCP;
		c.tcpconn.state = ESPCONN_NONE;
		c.tcpconn.proto.tcp = &c.esptcp;
		c.tcpconn.proto.tcp->local_port = 1288;
		espconn_regist_connectcb(&c.tcpconn, tcp_listen);
		espconn_accept(&c.tcpconn);
	    
    	c.udpconn.type = ESPCONN_UDP;
    	c.udpconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    	c.udpconn.proto.udp->local_port = 1288;
    	espconn_regist_recvcb(&c.udpconn, udp_callback);
    	espconn_create(&c.udpconn);

	    os_printf("%12u %12u UDP setup complete -  %d/%d\n\r", t/100000, ntp_unix_timestamp, wifiStatus, bootstatus);
	} 
	
}


void CP_ICACHE_FLASH_ATTR
doButton(uint8 gpio_pin)
{
	uint8 i = gpio_pin;	
	os_printf ( " ==> Starting doButton %d \n", i );		
	switch(i) {
		case 13:
			os_printf ( " ==> Pin %d pressed - Unpairing \n", i );	
			chatFabric_pair_init(&pair[0]);				
		break;
		default:
			os_printf ( " ==> Pin %d pressed \n", i );			
		break;
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
userWifiInit()
{
    //Set station mode
    wifi_set_opmode( 0x1 );

    //Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    os_timer_disarm(&boottimer);
    os_timer_setfn(&boottimer, (os_timer_func_t *)startup_station, NULL);
    os_timer_arm(&boottimer, 250, 1);
	shiftReg0();
}

void CP_ICACHE_FLASH_ATTR
userGPIOInit()
{

    gpio_init();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);

    
	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH(buttonPress, NULL);
	GPIO_DIS_OUTPUT(13); // set for input
	GPIO_DIS_OUTPUT(14); // set for input
	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(0); // set for input
//	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(3); // set for input

    gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_NEGEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(14), GPIO_PIN_INTR_NEGEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_NEGEDGE);    
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

	for (i=0; i<16; i++) {
		cpStatus[i] = -1;
	}

	for (i=0; i<4096; i++ ){
		flashConfig[i] = '\0';
	}

	heap =0;
	heapLast = 0;
		

}

//Init function 
void CP_ICACHE_FLASH_ATTR
user_init()
{
	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	uart0enabled = 1;
	userGPIOInit();
	shiftReg0();
	shiftReg1();

	os_timer_disarm(&statusReg);
	os_timer_setfn(&statusReg, (os_timer_func_t *)statusLoop, NULL);
	os_timer_arm(&statusReg, 250, 1);

	
    // uart_init(BIT_RATE_115200,BIT_RATE_115200);
    // Initialize the GPIO subsystem.
	chatFabricInit();
	userWifiInit();
}
