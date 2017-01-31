//////////////////////////////////////////////////
// rBoot sample project.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <c_types.h>
#include <osapi.h>
#include <user_interface.h>
#include <time.h>
#include <mem.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "esp8266.h"
#include "util.h"
#include "esp-cf-config.h"
#include "esp-cf-wifi.h"

#include "shell.h"

#include "rboot-api.h"
#include "rboot-ota.h"

#define BUFF_LEN 64
char shellBuffer[BUFF_LEN] = {0};
int8 sbi = 0;
static os_timer_t network_timer;
extern chatFabricConfig config;  

void ICACHE_FLASH_ATTR user_rf_pre_init() {
}

void ICACHE_FLASH_ATTR ShowIP() {
	struct ip_info ipconfig;
	char msg[50];
	wifi_get_ip_info(STATION_IF, &ipconfig);
	if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
		os_sprintf(msg, "ip: %d.%d.%d.%d, mask: %d.%d.%d.%d, gw: %d.%d.%d.%d\r\n",
			IP2STR(&ipconfig.ip), IP2STR(&ipconfig.netmask), IP2STR(&ipconfig.gw));
	} else {
		os_sprintf(msg, "network status: %d\r\n", wifi_station_get_connect_status());
	}
	os_printf(msg);
}

void ICACHE_FLASH_ATTR ShowInfo() {
	char msg[50];

    os_sprintf(msg, "\r\nSDK: v%s\r\n", system_get_sdk_version());
    os_printf(msg);

    os_sprintf(msg, "Free Heap: %d\r\n", system_get_free_heap_size());
    os_printf(msg);

    os_sprintf(msg, "CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    os_printf(msg);

    os_sprintf(msg, "System Chip ID: 0x%x\r\n", system_get_chip_id());
    os_printf(msg);

    os_sprintf(msg, "SPI Flash ID: 0x%x\r\n", spi_flash_get_id());
    os_printf(msg);

    os_sprintf(msg, "SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
    os_printf(msg);
}

void ICACHE_FLASH_ATTR Switch() {
	char msg[50];
	uint8 before, after;
	before = rboot_get_current_rom();
	if (before == 0) after = 1; else after = 0;
	os_sprintf(msg, "Swapping from rom %d to rom %d.\r\n", before, after);
	os_printf(msg);
	rboot_set_current_rom(after);
	os_printf("Restarting...\r\n\r\n");
	system_restart();
}

static void ICACHE_FLASH_ATTR OtaUpdate_CallBack(bool result, uint8 rom_slot) {

	if(result == true) {
		// success
		if (rom_slot == FLASH_BY_ADDR) {
			os_printf("Write successful.\r\n");
		} else {
			// set to boot new rom and then reboot
			char msg[40];
			os_sprintf(msg, "Firmware updated, rebooting to rom %d...\r\n", rom_slot);
			os_printf(msg);
			rboot_set_current_rom(rom_slot);
			system_restart();
		}
	} else {
		// fail
		os_printf("Firmware update failed!\r\n");
	}
}

static void ICACHE_FLASH_ATTR OtaUpdate() {
	
	// start the upgrade process
	if (rboot_ota_start((ota_callback)OtaUpdate_CallBack)) {
		os_printf("Updating...\r\n");
	} else {
		os_printf("Updating failed!\r\n\r\n");
	}
	
}

void shellCircleBuffer (char RcvChar) {

		if ( sbi == BUFF_LEN-1 ) {
			sbi = 0;
			shellBuffer[sbi] = 0;
			os_printf( " ===> Serial in buffer overflow. \n " );
			return;
		}
		if ( RcvChar == 8  || RcvChar == 127)  // backspace & delete 
		{
			// sbi - next open space
			sbi--; // pervious char
			shellBuffer[sbi] = 0; // delete the char
			// do not increment 
			return;
		}
		
        if (RcvChar == '\r') return;

		if (RcvChar == '\n' ) {
			shellBuffer[sbi] = 0;
//			os_printf( " ===> Serial: %d %s \n ", sbi, shellBuffer );
			ProcessCommand((char*)&shellBuffer);
			sbi = 0;
			shellBuffer[sbi] = 0;			
		} else {
			shellBuffer[sbi] = RcvChar;
			sbi++;
		}

	return;
}

void ICACHE_FLASH_ATTR ProcessCommand(char* str) {
	if (!strcmp(str, "help")) {
		os_printf(" =============================================== \n");	
		os_printf("chatFabric comnplie date: %s git:%s SDK Version: %s\n" , VERSION_DATE, VERSION_GIT, system_get_sdk_version() );
		os_printf("\n\n");	
		os_printf("available commands\r\n");
		os_printf("  help - display this message\r\n");
		os_printf("  ip - show current ip address\r\n");
		os_printf("  restart - restart the esp8266\r\n");
		os_printf("  switch - switch to the other rom and reboot\r\n");
		os_printf("  ota - perform ota update, switch rom and reboot\r\n");
		os_printf("  info - show esp8266 info\r\n");
		os_printf("\n");	
		os_printf("  === chatFabric controls ===\r\n");
		os_printf("  unpair - clear all chatFabric paining info\r\n");
		os_printf("  init - re-init all chatFabric config (factory reset)\r\n");
		os_printf("  debugoff - disable chatFabric debugging (DOES NOT SAVE CONFIG)\r\n");
		os_printf("  debugon - disable chatFabric debugging (DOES NOT SAVE CONFIG)\r\n");
		os_printf("  saveconfig - Saves the config into flash memory\r\n");
		os_printf("  readconfig - re-read the config into from flash memory to RAM.\r\n");
		os_printf("  raw0 - read from flash memory to RAM, print to hex\r\n");
		os_printf("  raw1 - read from flash memory to RAM, print to hex\r\n");
		os_printf("  raw2 - read from flash memory to RAM, print to hex\r\n");
		os_printf("  raw3 - read from flash memory to RAM, print to hex\r\n");
		os_printf("\r\n");
	} else if (str[0] == 0) {
		return;
	} else if (!strcmp(str, "ota")) {
		OtaUpdate();
	} else if (!strcmp(str, "restart")) {
		os_printf("Restarting...\r\n\r\n");
		system_restart();
	} else if (!strcmp(str, "ip")) {
		ShowIP();
	} else if (!strcmp(str, "info")) {
		ShowInfo();
	} else if (!strcmp(str, "unpair")) {	
		cfPairInit(&pair[0]);
		currentMode = MODE_STA_UNPAIRED;
		os_printf ( " ==> Unpaired !\n");
	} else if (!strcmp(str, "init")) {	
		currentMode = MODE_BOOTING;

		bzero(&flashConfig, 4096);
		cfConfigWrite(&config);
		
		chatFabricInit();
		createHostMeta();
		espCfConfigInit();
		cfConfigWrite(&config);

		system_restart();		
	} else if (!strcmp(str, "debugon")) {	
		_GLOBAL_DEBUG = 1;
		config.debug = 1;
	} else if (!strcmp(str, "debugoff")) {	
		_GLOBAL_DEBUG = 0;
		config.debug = 0;	
	} else if (!strcmp(str, "saveconfig")) {	
		cfConfigWrite(&config);
	} else if (!strcmp(str, "readconfig")) {			
		cfConfigRead(&config);		
		_GLOBAL_DEBUG = config.debug;
	} else if (!strcmp(str, "raw0")) {		
		espCfConfigRawRead(0);
	} else if (!strcmp(str, "raw1")) {		
		espCfConfigRawRead(1);
	} else if (!strcmp(str, "raw2")) {		
		espCfConfigRawRead(2);
	} else if (!strcmp(str, "raw3")) {		
		espCfConfigRawRead(3);
	} else  {	
		os_printf ( " ==> Command Unknown.\n");		
	}

	
}
