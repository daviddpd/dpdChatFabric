#include "esp-cf-config.h"

extern void* deviceCallBack;

enum deviceModes currentMode = MODE_UNDEFINED;
struct station_config stationConf;
unsigned char flashConfig[4096];
chatFabricConnection ESP_WORD_ALIGN c;
chatFabricConfig ESP_WORD_ALIGN config;  
msgbuffer ESP_WORD_ALIGN b;

void CP_ICACHE_FLASH_ATTR
espCfConfigInit()
{


	cfConfigInit(&config);
	config.wifi_ap_switch = 1;
	config.wifi_sta_switch = 0;
	cfConfigRead(&config);

	config.pairfile = "1";		
	config.callback = (void*)&deviceCallBack;
	config.debug = 1;

	config.numOfControllers = 4;
	config.controlers = (cfControl*)malloc(config.numOfControllers * sizeof(cfControl));
	
	memcpy( &config.wifi_ap_passwd, "esp8266!demo", 12 );
	memcpy( &config.wifi_sta_passwd, SSID_PASSWORD, strlen(SSID_PASSWORD) );
	memcpy( &config.wifi_sta_ssid, SSID, strlen(SSID) );

	// 13 == red
	// 12 == green
	// 4 == yellow

	int i =	0;		
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_GAUGE;
	config.controlers[i].value = 0;
	config.controlers[i].label = "Time";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 0xffffffff;			
	
	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].label = "yellow";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);

	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;

	config.controlers[i].gpio = 16;

	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].label = "green";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);

	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 13;
	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].label = "red";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);

	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;
	config.controlers[i].gpio = 12;
	
	config.mode = SOFTAP_MODE;
	config.mode = STATION_MODE;
	
/*
	i =	1;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_BOOLEAN;
	config.controlers[i].value = 0;
	config.controlers[i].label = "switch2";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);

	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 1;

	i++;
	config.controlers[i].control = i;
	config.controlers[i].type = ACTION_TYPE_DIMMER;
	config.controlers[i].value = 0;
	config.controlers[i].label = "Dimmer0";
	config.controlers[i].labelLength = strlen(config.controlers[i].label);
	config.controlers[i].rangeLow= 0;
	config.controlers[i].rangeHigh= 8;

*/


}