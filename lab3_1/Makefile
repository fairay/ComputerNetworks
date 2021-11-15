CPPFLAGS=-g -pthread -I/sw/include/root
LDFLAGS=-g
LDLIBS=-L/sw/lib/root -lCore
SRCS=server.cpp client.cpp dir_explore.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
EXES=server client

all: server client

server: server.o dir_explore.o
	g++ $(LDFLAGS) -o server server.o dir_explore.o

client: client.o
	g++ $(LDFLAGS) -o client client.o

server.o: server.cpp common.h
	g++ $(CPPFLAGS) -c server.cpp

dir_explore.o: dir_explore.cpp
	g++ $(CPPFLAGS) -c dir_explore.cpp

client.o: client.cpp common.h
	g++ $(CPPFLAGS) -c client.cpp

RM=rm -f
clean:
	$(RM) $(OBJS) $(EXES)
