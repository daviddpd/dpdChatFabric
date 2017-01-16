#ifndef __CHACHA20_H
#define __CHACHA20_H

#ifdef ESP8266
#include <c_types.h>
#else
#include <stdint.h>
#endif
#include <string.h>

void ChaCha20XOR(uint8_t key[32], uint32_t counter, uint8_t nonce[12], uint8_t *input, uint8_t *output, int inputlen);

#endif
