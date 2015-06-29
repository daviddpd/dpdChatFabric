CC = cc
CFLAGS= -g -O0 -c -Wall
INCLUDES=-I/usr/include -I/usr/local/include
LDFLAGS=-L/usr/lib -L/usr/local/lib
LOCALLIBS=dpdChatFabric.c
SRCS=dpdChatFabric-server.c dpdChatFabric-client.c
DEPS=$(LOCALLIBS:.c=.o)
OBJS=$(SRCS:.c=.o) $(DEPS)
EXECUTABLE=dpdChatFabric-server dpdChatFabric-client
LIBS= -lucl

CFLAGS  = -g -Wall  -I/usr/local/include -I/z/home/dpd/github/dpdChatFabric

all: $(OBJS) $(EXECUTABLE)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o $(DEPS) 
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $^


clean:
	rm -f *.o $(EXECUTABLE)

