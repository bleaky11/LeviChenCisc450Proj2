CC = gcc
CFLAGS = -Wall

all: UDPEchoClient UDPEchoServer

UDPEchoClient: UDPEchoClient.o DieWithError.o
	$(CC) $(CFLAGS) -o UDPEchoClient UDPEchoClient.o DieWithError.o -lpthread

UDPEchoServer: UDPEchoServer.o DieWithError.o
	$(CC) $(CFLAGS) -o UDPEchoServer UDPEchoServer.o DieWithError.o

DieWithError.o: DieWithError.c
	$(CC) $(CFLAGS) -c DieWithError.c

UDPEchoClient.o: UDPEchoClient.c
	$(CC) $(CFLAGS) -c UDPEchoClient.c

UDPEchoServer.o: UDPEchoServer.c
	$(CC) $(CFLAGS) -c UDPEchoServer.c

clean:
	rm -f UDPEchoClient.o DieWithError.o UDPEchoServer.o UDPEchoClient UDPEchoServer