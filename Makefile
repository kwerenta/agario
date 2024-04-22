CC			:= clang
CFLAGS	:= -Wall

SERVER_DIR	:= server
CLIENT_DIR	:= client
SHARED_DIR  := shared

SERVER_SOURCES := $(wildcard $(SERVER_DIR)/*.c)
CLIENT_SOURCES := $(wildcard $(CLIENT_DIR)/*.c)
SHARED_SOURCES := $(wildcard $(SHARED_DIR)/*.c)

.PHONY: client server

all: client server

client:
	$(CC) $(CFLAGS) -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 $(CLIENT_SOURCES) $(SHARED_SOURCES) -o client.o

server:
	$(CC) $(CFLAGS) $(SERVER_SOURCES) $(SHARED_SOURCES) -o server.o

clean:
	rm client.o server.o

