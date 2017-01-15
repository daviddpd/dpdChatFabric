#ifndef _shell_h_
#define _shell_h_

void ICACHE_FLASH_ATTR user_rf_pre_init();
void ICACHE_FLASH_ATTR ShowIP();
void ICACHE_FLASH_ATTR ShowInfo();
void ICACHE_FLASH_ATTR Switch();
static void ICACHE_FLASH_ATTR OtaUpdate_CallBack(bool result, uint8 rom_slot);
static void ICACHE_FLASH_ATTR OtaUpdate();
void ICACHE_FLASH_ATTR ProcessCommand(char* str);
void ICACHE_FLASH_ATTR startShell(void);


#endif