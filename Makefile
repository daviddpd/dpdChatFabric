CC = cc
CFLAGS= -g -O0 -c -Wall
INCLUDES=-I/usr/include -I/usr/local/include
#LDFLAGS=-L/usr/lib -L/usr/local/lib  -Wl,-rpath=/usr/local/lib/gcc48
LDFLAGS=-L/usr/lib -L/usr/local/lib  
LOCALLIBS=dpdChatFabric.c dpdChatPacket.c
SRCS=dpdChatFabric-server.c dpdChatFabric-client.c device.c createConfig.c controller.c
DEPS=$(LOCALLIBS:.c=.o)
OBJS=$(SRCS:.c=.o) $(DEPS)
EXECUTABLE=dpdChatFabric-server dpdChatFabric-client device createConfig controller
LIBS=-lsodium

CFLAGS  = -g -Wall  -I/usr/local/include -I/z/home/dpd/github/dpdChatFabric

all: $(OBJS) $(EXECUTABLE)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS) 
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $^


clean:
	rm -f *.o $(EXECUTABLE)

