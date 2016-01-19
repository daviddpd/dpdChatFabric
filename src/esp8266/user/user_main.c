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
#include "driver/spi.h"

//extern enum deviceModes currentMode;

extern time_t ntp_unix_timestamp;
time_t seconds_since_boot;
extern enum NTP_STATE ntp_status;
//extern char macAddr[];
extern hostmeta_t hostMeta;

extern char ntp_status_str[];
extern struct mdns_info *mdnsinfo;

int bootstatus = 0;  // 1 - network up, 2-ready
// os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop();
static void clock_loop();
void CP_ICACHE_FLASH_ATTR user_init_stage2();
//uint32_t ninc = 0;
unsigned char ch = 0x00;

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
uint32_t ntpcounter = 0;
uint32_t ntpstatus_printed = 0;


LOCAL os_timer_t boottimer;
LOCAL os_timer_t poketimer;

LOCAL os_timer_t buttonDebounce;
LOCAL os_timer_t shiftReg;
LOCAL os_timer_t statusReg;
LOCAL os_timer_t ntpTimer;
LOCAL os_timer_t clockTimer;
LOCAL os_timer_t sleepTimer;



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
//void shiftReg0();
//void shiftReg1();
//void changeMode(enum deviceModes m);
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
//		changeMode(menuItem);
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
//				shiftReg0();
//				shiftReg1();

				currentMode = MODE_BOOTING;
//				changeMode(MODE_BOOTING);
	
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
//				changeMode(MODE_STA_UNPAIRED);				
				os_printf ( " ==> Unpaired : %d \n", menuItem );
			break;
		}
		

		menuItem = MODE_MENU_NONE;
//		changeMode(currentMode);

	}
	
}







void CP_ICACHE_FLASH_ATTR 
gpioInitFromConfig(chatFabricConfig *config) 
{

	int i;
	for (i=0; i<config->numOfControllers; i++) 
	{
		if ( config->controlers[i].gpio >= 0 ) {
			if (config->controlers[i].gpio == 16) {
				gpio16_output_set(config->controlers[i].value ^ config->controlers[i].value_mask);
			} else {
				GPIO_OUTPUT_SET(config->controlers[i].gpio, config->controlers[i].value ^ config->controlers[i].value_mask );
			}		
		}
		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "=== %10s: %4d %24s %4d %4d", "Setting", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].gpio );
	}			

}

void CP_ICACHE_FLASH_ATTR 
deviceCallBack(chatFabricConfig *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd) 
{

	int i=0, x=0;

	unsigned char *tmp;

	CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "Start" );
	
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

				if  ( 	config->controlers[i].type == ACTION_TYPE_BOOLEAN ) {
					if (config->controlers[i].gpio == 16) {
						gpio16_output_set(config->controlers[i].value ^ config->controlers[i].value_mask );
					} else {
						GPIO_OUTPUT_SET(config->controlers[i].gpio, config->controlers[i].value ^ config->controlers[i].value_mask);
						CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "=== %10s: %4d %24s %4d %4d %4d", "Setting", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].value ^ config->controlers[i].value_mask, config->controlers[i].gpio );
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
//					shiftReg0();
				}				
			}
		}
		// CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "=== %10s: %4d %24s %4d %4d", "Control", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].gpio );
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
//	printf ( "Got network Packet.\n" );
//    os_printf("%12u %12u  GOT A UDP PACKET\n\r", t/100000, ntp_unix_timestamp);

    if (data == NULL) {
        return;
    }
    
    c.conn = arg;
    payloadMsg.length = 0;
    payloadMsg.msg = 0;
    mbuff.length = (int)length;
    mbuff.msg = data;

//	printf ( "chatFabric_device call.\n" );
    e = chatFabric_device(&c, &pair[0], &config,  &payloadMsg);
//	printf( "chatFabric_device return.\n" );

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
//	printf( "udp_callback return.\n" );
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
ntpTimer_loop()
{
//	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "%d;  ntp_status: %d;  ntpcounter : %d", ntp_unix_timestamp, ntp_status, ntpcounter );

	if ( ntp_status != NTP_STATE_SETTING ) {
		ntpcounter+=10;
	}

	if (ntp_status == NTP_STATE_TIMEOUT) {
		if ( ntpstatus_printed == 0 ) {
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "%s", ntp_status_str );
			ntpstatus_printed = 1;
		}
	}

	if ( 
			( (ntpcounter >= 1*60) && (ntp_status == NTP_STATE_TIMEOUT) ) ||
			( (ntpcounter >= 30*60) && (ntp_status == NTP_STATE_SET) ) 
	)		
	 {
		ntpcounter = 0;
		ntp_status = NTP_STATE_SETTING;
		ntp_get_time();
	}	
	
}

static void CP_ICACHE_FLASH_ATTR
clock_loop()
{

	if (ntp_unix_timestamp > 0) {
	    ntp_unix_timestamp++;
		if ( ( ntp_unix_timestamp % 60) == 0 ) { 
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "%d", ntp_unix_timestamp ); 
		}
	}

	seconds_since_boot++;

	
}

void CP_ICACHE_FLASH_ATTR
statusLoop() {
	uuid_cp u1;
	
	if ( pair[0].hasPublicKey && ( menuItem == MODE_MENU_NONE ) ) {
		currentMode = MODE_STA_PAIRED;
//		changeMode(MODE_STA_PAIRED);
	} 

//	shiftReg1();
}


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

//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
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
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U);

	GPIO_DIS_OUTPUT(0); // set for input
	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(0); // set for input
//	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(3); // set for input

//    gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_NEGEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_POSEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_POSEDGE);    
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
adc() {
	if ( 
		hostMeta.hwaddr[0] == 0x18 
		&& hostMeta.hwaddr[1] == 0xfe 
		&& hostMeta.hwaddr[2] == 0x34 
		&& hostMeta.hwaddr[3] == 0xd4 
		&& hostMeta.hwaddr[4] == 0xd3
		&& hostMeta.hwaddr[5] == 0x1d
	
	) {

	int i=0;
	uint16 data0;
	uint16 data1;
	uint32 data0sum = 0;
	uint32 data1sum = 0;
	
	for (i=0; i<20; i++) {
	
		os_timer_disarm(&poketimer);
		data0 = (uint16) spi_transaction(
			HSPI, 
			0, 
			0,
			0, 
			0, 
			4,
			0b1101, 
			12, 
			0);
		
		data0sum+=data0;
		
		data1 = (uint16) spi_transaction(
			HSPI, 
			0, 
			0,
			0, 
			0, 
			4,
			0b1111, 
			12, 
			0);
		data1sum+=data1;

		}
		
//		double voltsPerUnit = 5.22 / 2047;
		double voltsPerUnit = 25.61 /10000;
		uint16 voltsPerUnit_i = (uint16)voltsPerUnit;
		uint16 voltsPerUnit_f = (uint16)((voltsPerUnit - voltsPerUnit_i) *1000);

		double data0volt  = (data0sum/20)*voltsPerUnit;
		uint16 data0volt_i  = (uint16)data0volt;
		uint16 data0volt_f  = (uint16)((data0*voltsPerUnit - data0volt_i ) *1000);


		double data1volt  = (data1sum/20)*voltsPerUnit;
		uint16 data1volt_i  = (uint16)data1volt;
		uint16 data1volt_f  = (uint16)((data1*voltsPerUnit - data1volt_i ) *1000);


		CHATFABRIC_DEBUG_FMT(1, "[perUnit: %02u.%03u]  ADC0: %04x : volts  %02u.%03u : ADC1: %04x : volts  %02u.%03u ",voltsPerUnit_i, voltsPerUnit_f,  data0,  data0volt_i, data0volt_f, data1, data1volt_i, data1volt_f);
		
		os_timer_setfn(&poketimer, (os_timer_func_t *)adc, NULL);
		os_timer_arm(&poketimer, 500, 1);
			
/*

		uint16 cmd = ;
		spi_tx16(HSPI, cmd);
		uint16 data = (uint16) spi_rx16(HSPI);
		CHATFABRIC_DEBUG_FMT(1, "ADC0: %04x", data ); 											

		cmd = 0b0111100000000000;
		spi_tx16(HSPI, cmd);
		data = (uint16) spi_rx16(HSPI);
		CHATFABRIC_DEBUG_FMT(1, "ADC1: %04x", data );
*/
	}
}



void CP_ICACHE_FLASH_ATTR
user_init_stage2() 
{
	chatFabricInit();

	createHostMeta();
	espCfConfigInit();
	cfConfigWrite(&config);
	
	cfPairInit(&pair[0]);
	if ( flashConfig[2048] == cftag_header ) {
		CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "reading pair config\n");
		cfPairRead(&config, (chatFabricPairing *)&(pair[0]) );
	}
	if ( config.hasPairs ) {
		currentMode = MODE_STA_PAIRED;
//		changeMode(MODE_STA_PAIRED);
	} else {
		currentMode = MODE_STA_UNPAIRED;
//		changeMode(MODE_STA_UNPAIRED);
	}

	char buf[38];
	snprintf_uuid(buf, sizeof(buf), &config.uuid.u0);
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	snprintf_uuid(buf, sizeof(buf), &config.uuid.u1);
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	espWiFiInit();
//	shiftReg0();
//	shiftReg1();

//	cfConfigWrite(&config);
	_GLOBAL_DEBUG = config.debug;
	gpioInitFromConfig(&config);

//	os_timer_disarm(&statusReg);
//	os_timer_setfn(&statusReg, (os_timer_func_t *)statusLoop, NULL);
//	os_timer_arm(&statusReg, 300, 1);
	if ( 
		hostMeta.hwaddr[0] == 0x18 
		&& hostMeta.hwaddr[1] == 0xfe 
		&& hostMeta.hwaddr[2] == 0x34 
		&& hostMeta.hwaddr[3] == 0xd4 
		&& hostMeta.hwaddr[4] == 0xd3
		&& hostMeta.hwaddr[5] == 0x1d
	
	) {
	
		spi_init(HSPI);
		os_timer_disarm(&poketimer);
		os_timer_setfn(&poketimer, (os_timer_func_t *)adc, NULL);
		os_timer_arm(&poketimer, 500, 1);
	}


	
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
	seconds_since_boot=0;
	uart_init(BIT_RATE_115200,BIT_RATE_115200);

	currentMode = MODE_BOOTING;
	uart0enabled = 1;
	userGPIOInit();
	user_init_stage2();


	os_timer_disarm(&clockTimer);
	os_timer_setfn(&clockTimer, (os_timer_func_t *)clock_loop, NULL);
	os_timer_arm(&clockTimer, 1000, 1);

	os_timer_disarm(&ntpTimer);
	os_timer_setfn(&ntpTimer, (os_timer_func_t *)ntpTimer_loop, NULL);
	os_timer_arm(&ntpTimer, 10000, 1);


}