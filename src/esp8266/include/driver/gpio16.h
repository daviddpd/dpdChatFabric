#ifndef __GPIO16_H__
#define __GPIO16_H__

#ifdef ESP8266
//#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#ifndef CP_ICACHE_FLASH_ATTR
#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#endif
#endif

void gpio16_output_conf(void);
void gpio16_output_set(uint8 value);
void gpio16_input_conf(void);
uint8 gpio16_input_get(void);

#endif
