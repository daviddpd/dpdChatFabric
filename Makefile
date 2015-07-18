CC = cc
#CC = gcc48
#CFLAGS= -g -O0 -c -Wall  -finstrument-functions
CFLAGS  = -g  -O0 -Wall 
INCLUDES=-I/usr/include -I/usr/local/include -I/z/home/dpd/github/dpdChatFabric 
#LDFLAGS=-L/usr/lib -L/usr/local/lib  -Wl,-rpath=/usr/local/lib/gcc48 -pg 
LDFLAGS=-L/usr/lib -L/usr/local/lib  
LOCALLIBS=dpdChatFabric.c dpdChatPacket.c 
LIBS=
DEFS=

ifdef HAVE_SODIUM
DEFS+=-DHAVE_SODIUM
LIBS+=-lsodium
endif

ifdef HAVE_LOCAL_CRYPTO
DEFS+=-DHAVE_LOCAL_CRYPTO
INCLUDES+= -I./crypto
LOCALLIBS+=crypto/chacha20_simple.c crypto/curve25519-donna.c crypto/poly1305-donna.c crypto/salsa20.c
endif

SRCS=dpdChatFabric-server.c dpdChatFabric-client.c device.c createConfig.c controller.c examples/crypto-test.c
DEPS=$(LOCALLIBS:.c=.o)
OBJS=$(SRCS:.c=.o) $(DEPS)
EXECUTABLE=dpdChatFabric-server dpdChatFabric-client device createConfig controller examples/crypto-test
#LIBSSTATIC=/usr/local/lib/libsodium.a
LIBSSTATIC=


all: $(OBJS) $(EXECUTABLE)

examples/crypto-test: examples/crypto-test.o crypto/chacha20_simple.o crypto/curve25519-donna.o crypto/poly1305-donna.o crypto/salsa20.o
	$(CC) $(LDFLAGS) -lsodium $(LIBS) $(DEFS) -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) $(DEFS)

%: %.o $(DEPS) 
	$(CC) $(LDFLAGS) $(LIBS) $(DEFS) -o $@ $^

clean:
	rm -f *.o crypto/*.o examples/*.o $(EXECUTABLE)

