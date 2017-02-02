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
#include "driver/spi.h"
#include <sys/time.h>
#include "uuuid2.h"
#include <c_types.h>
#include <sys/types.h>

#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
#define PWM_0_OUT_IO_NUM 14
#define PWM_0_OUT_IO_FUNC  FUNC_GPIO14

#define PWM_1_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM 15
#define PWM_1_OUT_IO_FUNC  FUNC_GPIO15

//extern enum deviceModes currentMode;

extern time_t ntp_unix_timestamp;
time_t seconds_since_boot;
extern enum NTP_STATE ntp_status;
//extern char macAddr[];
extern hostmeta_t hostMeta;

extern char ntp_status_str[];
extern struct mdns_info *mdnsinfo;

extern void ProcessCommand(char* str);

int bootstatus = 0;  // 1 - network up, 2-ready
// os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop();
static void clock_loop();
void CP_ICACHE_FLASH_ATTR user_init_stage2();
//uint32_t ninc = 0;
unsigned char ch = 0x00;

int uart0enabled = -1;
int shiftCounter = -1;
//uint8 shiftBits0[8];
//uint8 shiftBits1[8];
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

LOCAL os_timer_t buttonDebounce;
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
//void changeMode(enum deviceModes m);
enum deviceModes menuItem = MODE_MENU_NONE;
void adcBultin();
void adcCallBack();


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
				currentMode = MODE_BOOTING;
//				changeMode(MODE_BOOTING);
	
				os_timer_disarm(&statusReg);
				os_timer_setfn(&statusReg, (os_timer_func_t *)statusLoop, NULL);
				os_timer_arm(&statusReg, 500, 1);
	
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
//	uint32 duty = 0;
	uint32 period = 0;
	uint32 period_max =0;;
	uint32 perunit =0;
	
	unsigned char *tmp;

	CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "Start" );
	
	for (i=0; i<config->numOfControllers; i++) 
	{
		if ( config->controlers[i].control == cp->action_control ) 
		{
	
			if (cp->action == ACTION_GET ) 
			{
				if ( config->controlers[i].readFuction != 0 )
				{
					CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "Starting readFunction callback" );
					config->controlers[i].readFuction(&(config->controlers[i]));
					CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "Done readFunction callback" );
				}

				reply->action = ACTION_READ;
				reply->action_control = cp->action_control;
				reply->action_type = config->controlers[i].type;
				reply->action_value = config->controlers[i].value;
				reply->action_length = 0;
		
		
			} else if (cp->action == ACTION_SET && config->controlers[i].readOnly != 1 ) 
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
						CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "=== %10s: %4d %-24s %4d %4d %4d", "Setting", config->controlers[i].control, config->controlers[i].label, config->controlers[i].value, config->controlers[i].value ^ config->controlers[i].value_mask, config->controlers[i].gpio );
					}
				}
				if  ( 	config->controlers[i].type == ACTION_TYPE_DIMMER ) 
				{					
			
					x = config->controlers[i].value_mask;

//					duty[x] = config->controlers[i].value;
/*					period = pwm_get_period();
					period_max = ((period * 1000) / 45);
	
					perunit = period_max/100; // 100%
					
					if ( config->controlers[i].value < 1 ) {
						duty =0;
					} else if ( config->controlers[i].value > 99 ) {
						duty = 22222; 
					} else {
						duty =  config->controlers[i].value * perunit;
					}

					
					CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "=== %10s: %4d %-24s %4d %4d Duty: %d %d %d %d", "Control (Dimmer)", 
						config->controlers[i].control, 
						config->controlers[i].label, 
						config->controlers[i].gpio,
						config->controlers[i].value, 
						(uint32)duty,
						(uint32)period,
						(uint32)period_max,
						(uint32)perunit
						);
					
					pwm_set_duty( duty, 1 );
					pwm_start();
*/
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
	uuuid2_t uuid;
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
clock_loop()
{
	
	if (ntp_unix_timestamp > 0) {
	    ntp_unix_timestamp++;		
	}
	seconds_since_boot++;
	
}


void CP_ICACHE_FLASH_ATTR
statusLoop() {
	os_timer_disarm(&statusReg);

	if ( pair[0].hasPublicKey && ( menuItem == MODE_MENU_NONE ) ) {
		currentMode = MODE_STA_PAIRED;
	} 
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

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10);

//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
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
	GPIO_DIS_OUTPUT(4); // set for input
//	GPIO_DIS_OUTPUT(5); // set for input
//	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(3); // set for input

//    gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_NEGEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_POSEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_POSEDGE);    
//    gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_POSEDGE); 
    gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_POSEDGE);    

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

	bzero(&flashConfig, 4096);

//	heap =0;
//	heapLast = 0;
//	heapLast = heap;
//	heap = system_get_free_heap_size();
		

}

void CP_ICACHE_FLASH_ATTR
adcBultin() {

	uint16 v = 0;
	
	v = system_adc_read();
	
//		10mV /C  = 0.010 V/C = 1/100
		double voltsPerUnit = 1 /100;
		
		
		uint16 voltsPerUnit_i = (uint16)voltsPerUnit;
		uint16 voltsPerUnit_f = (uint16)((voltsPerUnit - voltsPerUnit_i) *100);

		double data0volt  = (v)*voltsPerUnit;
		uint16 data0volt_i  = (uint16)data0volt;
		uint16 data0volt_f  = (uint16)((v*voltsPerUnit - data0volt_i ) *100);


		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "[perUnit: %02u.%03u]  ADC0: %04x : volts  %02u.%03u   ", voltsPerUnit_i, voltsPerUnit_f,  v,  data0volt_i, data0volt_f);


}


void CP_ICACHE_FLASH_ATTR
adcCallBack(void * control ) {

	cfControl *z = (cfControl *)control;
	uint16 a,b,c,v = 0;
	a = system_adc_read();
	os_delay_us(1000);
	b = system_adc_read();
	os_delay_us(1000);
	c = system_adc_read();
	v = ( a + b + c ) / 3; 
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, " ADC0: %6d %6d %6d  ADC0_AVG: %6d", a, b, c, v);	
	z->value = v;

}


void CP_ICACHE_FLASH_ATTR
user_init_stage2() 
{
	int i;
	chatFabricInit();
	createHostMeta();
	espCfConfigInit();
	cfConfigWrite(&config);

	for (i=0; i<MAX_PAIRS; i++) {
		cfPairInit(&pair[i]);
	}
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

	uuuid2_to_str(&buf[0], sizeof(buf), &config.uuid.u0);	
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	uuuid2_to_str(&buf[0], sizeof(buf), &config.uuid.u1);	
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	espWiFiInit();

	_GLOBAL_DEBUG = config.debug;
	gpioInitFromConfig(&config);

	os_timer_disarm(&statusReg);
	os_timer_setfn(&statusReg, (os_timer_func_t *)statusLoop, NULL);
	os_timer_arm(&statusReg, 2000, 1);


	//startShell
	ProcessCommand("help");
	
}

//Init function 
void CP_ICACHE_FLASH_ATTR
user_init()
{
	int i;

	uart0enabled = 1;
	userGPIOInit();

	seconds_since_boot=0;
	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	#ifdef VERSION_DATE
	#ifdef VERSION_GIT
	for (i=0; i<256; i++) {
		os_printf(".");
		os_delay_us(10000);
	}
	os_printf("\n");	
	os_printf("chatFabric comnplie date: %s git:%s SDK Version: %s\n" , VERSION_DATE, VERSION_GIT, system_get_sdk_version() );
	CHATFABRIC_PRINT("\n");
	#endif
	#endif
	

	currentMode = MODE_BOOTING;

	user_init_stage2();

	hw_timer_init(0,1);
	hw_timer_set_func(clock_loop);
	hw_timer_arm(1000000);


}