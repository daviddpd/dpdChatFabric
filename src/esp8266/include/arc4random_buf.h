#ifndef arc4random_buf_h
#define arc4random_buf_h


#ifdef ESP8266
//#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#ifndef CP_ICACHE_FLASH_ATTR
#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#endif
#else
#define CP_ICACHE_FLASH_ATTR __attribute__(())
#endif

void arc4random_buf(unsigned char *b, int len );

#endif
