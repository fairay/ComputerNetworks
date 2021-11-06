CPPFLAGS=-g -pthread -I/sw/include/root
LDFLAGS=-g
LDLIBS=-L/sw/lib/root -lCore
SRCS=server.cpp client.cpp http.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
EXES=server client

all: server client

server: server.o http.o
	g++ $(LDFLAGS) -o server server.o http.o

client: client.o
	g++ $(LDFLAGS) -o client client.o

server.o: server.cpp common.h
	g++ $(CPPFLAGS) -c server.cpp

http.o: http.cpp http.hpp
	g++ $(CPPFLAGS) -c http.cpp

client.o: client.cpp common.h
	g++ $(CPPFLAGS) -c client.cpp

RM=rm -f
clean:
	$(RM) $(OBJS) $(EXES)
