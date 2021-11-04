CPPFLAGS=-g -pthread -I/sw/include/root
LDFLAGS=-g
LDLIBS=-L/sw/lib/root -lCore
SRCS=server.cpp client.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
EXES=$(subst .cpp, ,$(SRCS))

all: server client

server: server.o
	g++ $(LDFLAGS) -o server server.o

server.o: server.cpp common.h
	g++ $(CPPFLAGS) -c server.cpp

client: client.o
	g++ $(LDFLAGS) -o client client.o

client.o: client.cpp common.h
	g++ $(CPPFLAGS) -c client.cpp

RM=rm -f
clean:
	$(RM) $(OBJS) $(EXES)
