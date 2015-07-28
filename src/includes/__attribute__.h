
#ifndef cf___attribute__h
#define cf___attribute__h

#ifdef ESP8266
	#ifndef CP_ICACHE_FLASH_ATTR
		#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
	#endif

	#define ESP_WORD_ALIGN __attribute__ ((aligned (4)))
#else
	#define CP_ICACHE_FLASH_ATTR __attribute__(())
	#define ESP_WORD_ALIGN __attribute__ (())
#endif

#endif