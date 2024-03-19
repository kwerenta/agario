CC			:= clang
CFLAGS	:= -Wall

SERVER_DIR	:= server
CLIENT_DIR	:= client

SERVER_SOURCES := $(wildcard $(SERVER_DIR)/*.c)
CLIENT_SOURCES := $(wildcard $(CLIENT_DIR)/*.c)

.PHONY: client server

all: client server

client:
	$(CC) $(CFLAGS) $(CLIENT_SOURCES) -o client.o

server:
	$(CC) $(CFLAGS) $(SERVER_SOURCES) -o server.o

clean:
	rm client.o server.o

