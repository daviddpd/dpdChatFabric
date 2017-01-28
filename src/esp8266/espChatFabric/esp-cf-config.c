#include "esp-cf-config.h"

extern void* deviceCallBack;
extern void* adcCallBack;
extern hostmeta_t hostMeta;
enum deviceModes currentMode = MODE_UNDEFINED;
struct station_config stationConf;
unsigned char flashConfig[4096];
chatFabricConnection ESP_WORD_ALIGN c;
chatFabricConfig ESP_WORD_ALIGN config;  
msgbuffer ESP_WORD_ALIGN b;

#define RAW_READ 32

void CP_ICACHE_FLASH_ATTR
espCfConfigRawRead() {	
	unsigned char *tmp;
	char str[64] = {0};
	int i, sec;
	tmp = calloc(RAW_READ, sizeof(unsigned char)) ;
	
	for (sec=PRIV_PARAM_START_SEC; sec<PRIV_PARAM_START_SEC+3; sec++) {
		CHATFABRIC_DEBUG_FMT(1, " == Address : %05x", sec );
		for (i=0; i<SPI_FLASH_SEC_SIZE; i=i+RAW_READ) 
		{
			spi_flash_read(sec * SPI_FLASH_SEC_SIZE + i, (uint32 *)tmp, RAW_READ);
			os_sprintf (&str, "[FLASH %08x]", sec * SPI_FLASH_SEC_SIZE + i );
			util_bin2hex ( " ", (char*)&str, tmp, RAW_READ  );
//			CHATFABRIC_DEBUG_B2H(1, str, (unsigned char*)&(tmp[0]), RAW_READ  );
		}
	}

}


void CP_ICACHE_FLASH_ATTR
espCfConfigInit()
{


	cfConfigInit(&config);
	config.wifi_ap_switch = CONFIG_AP_SWITCH;
	config.wifi_sta_switch = CONFIG_STA_SWITCH;
	cfConfigRead(&config);
	

	config.pairfile = "1";		
	config.callback = (void*)&deviceCallBack;
	config.debug = 0;

	// FIXME: move to define/constant
	memcpy( &config.wifi_ap_passwd, "esp8266!demo", 12 );

#ifdef STA_DEFAULT_NETWORK
	memcpy( &config.wifi_sta_passwd, SSID_PASSWORD, strlen(SSID_PASSWORD) );
	memcpy( &config.wifi_sta_ssid, SSID, strlen(SSID) );
#endif

	// 13 == red
	// 12 == green
	// 4 == yellow

/*
if ( 
	hostMeta.hwaddr[0] == 0x18 
	&& hostMeta.hwaddr[1] == 0xfe 
	&& hostMeta.hwaddr[2] == 0x34 
	&& hostMeta.hwaddr[3] == 0xd4 
	&& hostMeta.hwaddr[4] == 0xd3
	&& hostMeta.hwaddr[5] == 0x1d
	
) {


	int i =	0;		
	config.numOfControllers = 1;
	config.controlers = (cfControl*)malloc(config.numOfControllers * sizeof(cfControl));

	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "yellow";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 16;


	i++;


		
	
	

} else 

*/
if ( 
	hostMeta.hwaddr[0] == 0x18 
	&& hostMeta.hwaddr[1] == 0xfe 
	&& hostMeta.hwaddr[2] == 0x34 
	&& hostMeta.hwaddr[3] == 0xa2 
	&& hostMeta.hwaddr[4] == 0xe0
	&& hostMeta.hwaddr[5] == 0x5b
	
) {
//#ifdef ESP_DEVICE_OUTLET

	int i =	0;		
	config.numOfControllers = 2;
	config.controlers = (cfControl*)malloc(config.numOfControllers * sizeof(cfControl));
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x01;
	config.controlers[i].label = "Top Outlet";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 12;

	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x01;	
	config.controlers[i].label = "Bottom Outlet";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 13;
	config.debug = 0;

} else if (1) {

	int i =	0;		

	config.debug = 1;

	config.numOfControllers = 5;
	
	config.controlers = (cfControl*)malloc(config.numOfControllers * sizeof(cfControl));
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "Top Outlet";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 12;
	config.controlers[i].readOnly = 0;
	config.controlers[i].readFuction = 0;
	
	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;	
	config.controlers[i].label = "Bottom Outlet";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 13;
	config.controlers[i].readOnly = 0;
	config.controlers[i].readFuction = 0;

	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_DIMMER;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;	
	config.controlers[i].label = "gpio14";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 100;
	config.controlers[i].gpio = 14;
	config.controlers[i].readOnly = 0;
	config.controlers[i].readFuction = 0;


	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "gpio16";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 16;
	config.controlers[i].readOnly = 0;
	config.controlers[i].readFuction = 0;

	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_GAUGE;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "ADC";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1024;			
	config.controlers[i].gpio = -1;
	config.controlers[i].readOnly = 1;
	config.controlers[i].readFuction = (void*)&adcCallBack;
	

} else {


	// ESP_DEVICE_GENERIC

	int i =	0;		
	config.numOfControllers = 4;
	config.controlers = (cfControl*)malloc(config.numOfControllers * sizeof(cfControl));
	
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_GAUGE;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "Time";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 0xffffffff;			
	config.controlers[i].gpio = -1;
	
	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "yellow";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 16;

	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "green";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 13;

	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "red";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 12;


	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].value_mask = 0x00;
	config.controlers[i].label = "red";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 12;

}


}