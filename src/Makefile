CC = cc
#CC = gcc48
CFLAGS  = -g  -O0 -Wall 
INCLUDES=-I/usr/include -I/usr/local/include -I./includes 
LDFLAGS=-L/usr/lib -L/usr/local/lib  

LOCAL_LIB_SRC=
DEFS=
LIBS=

ifdef FREEBSD
DEFS+=-DFREEBSD
endif

ifdef HAVE_SODIUM
DEFS+=-DHAVE_SODIUM
LIBS+=-lsodium
endif

ifdef HAVE_LOCAL_CRYPTO
DEFS+=-DHAVE_LOCAL_CRYPTO
INCLUDES+= -I./crypto
LOCAL_LIB_SRC+=crypto/chacha20_simple.c crypto/curve25519-donna.c crypto/poly1305-donna.c crypto/salsa20.c 
endif

LOCAL_LIB_SRC+=dpdChatFabric/dpdChatFabric.c dpdChatFabric/cfTagEncoder.c dpdChatFabric/cfConfig.c dpdChatFabric/cfPairConfig.c dpdChatPacket/dpdChatPacket.c util/util.c util/args.c uuid_wrapper/uuid_wrapper.c

LOCAL_LIB_OBJ=$(LOCAL_LIB_SRC:.c=.o)

BIN_SRC=bin/device.c bin/createConfig.c bin/controller.c  bin/cpTags.c
BIN_OBJ=$(BIN_SRC:.c=.o)

BIN_EXEC=$(BIN_SRC:.c=)

all:  $(BIN_OBJ) $(LOCAL_LIB_OBJ) $(BIN_EXEC)

examples/crypto-test: examples/crypto-test.o crypto/chacha20_simple.o crypto/curve25519-donna.o crypto/poly1305-donna.o crypto/salsa20.o
	$(CC) $(LDFLAGS) -lsodium $(LIBS) $(DEFS) -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) $(DEFS)

%: %.o $(LOCAL_LIB_OBJ) 
	$(CC) $(LDFLAGS) $(LIBS) $(DEFS) -o $@ $^

clean:
	rm -f $(BIN_EXEC) $(BIN_OBJ) $(LOCAL_LIB_OBJ)

