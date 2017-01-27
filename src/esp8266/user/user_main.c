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
#include "pwm.h"
#include <sys/time.h>
#include "uuuid2.h"
#include <c_types.h>
#include <sys/types.h>

//typedef uint32_t     time_t;

#define PWM_12_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define PWM_12_OUT_IO_NUM 12
#define PWM_12_OUT_IO_FUNC  FUNC_GPIO12

#define PWM_14_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
#define PWM_14_OUT_IO_NUM 14
#define PWM_14_OUT_IO_FUNC  FUNC_GPIO14

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

uint32 duty = 0;
int8 dimmerDirection = 1;
int8 dimmerLevel = 0;
int stepper = 0;
#define DIMMER_UNITS 20
#define DIMMER_INTERVAL 5000


LOCAL os_timer_t boottimer;
LOCAL os_timer_t poketimer;

LOCAL os_timer_t buttonDebounce;
LOCAL os_timer_t shiftReg;
LOCAL os_timer_t statusReg;
LOCAL os_timer_t ntpTimer;
LOCAL os_timer_t clockTimer;
LOCAL os_timer_t sleepTimer;
LOCAL os_timer_t blinkingTimer;



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
//				shiftReg0();
//				shiftReg1();

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
pwm_setup() 
{
    uint32 pwm_duty_init[1] = {0};

	uint32 io_info[][3] = { 
		{PWM_14_OUT_IO_MUX,PWM_14_OUT_IO_FUNC,PWM_14_OUT_IO_NUM}		
	};
	
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "io_info" );
    set_pwm_debug_en(0);//disable debug print in pwm driver
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "pwm debug" );
	
    /*PIN FUNCTION INIT FOR PWM OUTPUT*/
//    pwm_init(pwm_period,  pwm_duty_init ,0,io_info);

    pwm_init(800, pwm_duty_init ,1,io_info);
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,	 "pwm init" );
    
	pwm_start();
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "pwm start" );


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
	uint32 duty = 0;
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
			
					period = pwm_get_period();
					period_max = ((period * 1000) / 45);
	
					perunit = period_max/100; // 100%

					duty =  config->controlers[i].value * perunit;

					
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
					
					pwm_set_duty( duty, 0 );
					pwm_start();
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
//		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "%d", ntp_unix_timestamp ); 
		
	}
	seconds_since_boot++;
	
}


static void 
blinking_loop() {
	uint32 duty = 0;
	uint32 period = 0;
	uint32 period_max =0;;
	uint32 perunit =0;

	os_timer_disarm(&blinkingTimer);
		
		period = pwm_get_period();
		period_max = ((period * 1000) / 45);
		perunit = period_max/DIMMER_UNITS; // 100%

		duty =  dimmerLevel * perunit;
/*
		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "BLINKING Loop Period: %6d ; Period Max %6d ;  Duty %6d ; DimmerLevel %6d ; DimmerLevel %6d ; DimmerDirection %6d ", 
			period,
			period_max,
			duty,
			dimmerLevel,
			dimmerDirection
			); 
*/
							
		pwm_set_duty( duty, 0 );
		pwm_start();

		if ( dimmerDirection == 1 ) {
			dimmerLevel++;
		} else {
			dimmerLevel--;
		}
		
		if ( dimmerLevel > DIMMER_UNITS ) {
			dimmerLevel = DIMMER_UNITS-1;
			dimmerDirection = 0;
		} else if ( dimmerLevel < 0 ) {
			dimmerDirection = 1;
			dimmerLevel = 0;
		}

	os_timer_arm(&blinkingTimer, DIMMER_INTERVAL, 1);
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

//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
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
	GPIO_DIS_OUTPUT(5); // set for input
//	GPIO_DIS_OUTPUT(2); // set for input
//	GPIO_DIS_OUTPUT(3); // set for input

//    gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_NEGEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_POSEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_POSEDGE);    
    gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_POSEDGE); 
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

	for (i=0; i<4096; i++ ){
		flashConfig[i] = '\0';
	}

	heap =0;
	heapLast = 0;
	heapLast = heap;
	heap = system_get_free_heap_size();
		

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
// void CP_ICACHE_FLASH_ATTR
// adc() {
// 	if ( 
// 		hostMeta.hwaddr[0] == 0x18 
// 		&& hostMeta.hwaddr[1] == 0xfe 
// 		&& hostMeta.hwaddr[2] == 0x34 
// 		&& hostMeta.hwaddr[3] == 0xd4 
// 		&& hostMeta.hwaddr[4] == 0xd3
// 		&& hostMeta.hwaddr[5] == 0x1d
// 	
// 	) {
// 
// 	int i=0;
// 	uint16 data0;
// 	uint16 data1;
// 	uint32 data0sum = 0;
// 	uint32 data1sum = 0;
// 	
// 	for (i=0; i<20; i++) {
// 	
// 		os_timer_disarm(&poketimer);
// 		data0 = (uint16) spi_transaction(
// 			HSPI, 
// 			0, 
// 			0,
// 			0, 
// 			0, 
// 			4,
// 			0b1101, 
// 			12, 
// 			0);
// 		
// 		data0sum+=data0;
// 		
// 		data1 = (uint16) spi_transaction(
// 			HSPI, 
// 			0, 
// 			0,
// 			0, 
// 			0, 
// 			4,
// 			0b1111, 
// 			12, 
// 			0);
// 		data1sum+=data1;
// 
// 		}
// 		
// //		double voltsPerUnit = 5.22 / 2047;
// 		double voltsPerUnit = 25.61 /10000;
// 		uint16 voltsPerUnit_i = (uint16)voltsPerUnit;
// 		uint16 voltsPerUnit_f = (uint16)((voltsPerUnit - voltsPerUnit_i) *1000);
// 
// 		double data0volt  = (data0sum/20)*voltsPerUnit;
// 		uint16 data0volt_i  = (uint16)data0volt;
// 		uint16 data0volt_f  = (uint16)((data0*voltsPerUnit - data0volt_i ) *1000);
// 
// 
// 		double data1volt  = (data1sum/20)*voltsPerUnit;
// 		uint16 data1volt_i  = (uint16)data1volt;
// 		uint16 data1volt_f  = (uint16)((data1*voltsPerUnit - data1volt_i ) *1000);
// 
// 
// 		CHATFABRIC_DEBUG_FMT(1, "[perUnit: %02u.%03u]  ADC0: %04x : volts  %02u.%03u : ADC1: %04x : volts  %02u.%03u ",voltsPerUnit_i, voltsPerUnit_f,  data0,  data0volt_i, data0volt_f, data1, data1volt_i, data1volt_f);
// 		
// 		os_timer_setfn(&poketimer, (os_timer_func_t *)adc, NULL);
// 		os_timer_arm(&poketimer, 500, 1);
// 			
// /*
// 
// 		uint16 cmd = ;
// 		spi_tx16(HSPI, cmd);
// 		uint16 data = (uint16) spi_rx16(HSPI);
// 		CHATFABRIC_DEBUG_FMT(1, "ADC0: %04x", data ); 											
// 
// 		cmd = 0b0111100000000000;
// 		spi_tx16(HSPI, cmd);
// 		data = (uint16) spi_rx16(HSPI);
// 		CHATFABRIC_DEBUG_FMT(1, "ADC1: %04x", data );
// */
// 	}
// }



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

	uuuid2_to_str(&buf[0], sizeof(buf), &config.uuid.u0);	
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	uuuid2_to_str(&buf[0], sizeof(buf), &config.uuid.u1);	
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "UUID0 : %s", buf );
	
	espWiFiInit();

	_GLOBAL_DEBUG = config.debug;
	gpioInitFromConfig(&config);
	pwm_setup();

	os_timer_disarm(&statusReg);
	os_timer_setfn(&statusReg, (os_timer_func_t *)statusLoop, NULL);
	os_timer_arm(&statusReg, 2000, 1);


//	startShell();

	
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

	os_timer_disarm(&blinkingTimer);
	os_timer_setfn(&blinkingTimer, (os_timer_func_t *)blinking_loop, NULL);
	os_timer_arm(&blinkingTimer, DIMMER_INTERVAL, 1);

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
	uart0enabled = 1;
	userGPIOInit();
	user_init_stage2();


	os_timer_disarm(&clockTimer);
	os_timer_setfn(&clockTimer, (os_timer_func_t *)clock_loop, NULL);
	os_timer_arm(&clockTimer, 1000, 1);


}