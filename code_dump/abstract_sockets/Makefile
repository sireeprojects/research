CC = g++
CFLAGS = -Wall -std=c++11

all: server client

server: server.cpp
	$(CC) $(CFLAGS) server.cpp -o server

client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client

clean:
	rm -f server client /tmp/uds_example.sock
