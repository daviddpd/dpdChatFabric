#ifndef POLY1305_DONNA_H
#define POLY1305_DONNA_H

#include <stddef.h>

typedef struct poly1305_context {
	size_t aligner;
	unsigned char opaque[136];
} poly1305_context;

void poly1305_init(poly1305_context *ctx,  unsigned char *key);
void poly1305_update(poly1305_context *ctx, const unsigned char *m, size_t bytes);
void poly1305_finish(poly1305_context *ctx, unsigned char *mac);
void poly1305_auth(unsigned char *mac, const unsigned char *m, size_t bytes, unsigned char *key);

int poly1305_verify( unsigned char *mac1,  unsigned char *mac2);
int poly1305_power_on_self_test(void);

#endif /* POLY1305_DONNA_H */

