CC = g++
CFLAGS = -Wall -std=c++11

all: server client

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp -lpthread

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

clean:
	rm -f server client