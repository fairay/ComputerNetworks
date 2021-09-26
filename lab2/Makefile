all: client.o server.o

server.o: header.h server.c
	gcc -o server.o server.c
client.o: header.h client.c
	gcc -o client.o client.c

clean:	
	rm -f *.o
	rm -f *.soc
