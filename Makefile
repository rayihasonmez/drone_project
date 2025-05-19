CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11 -g `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf -lpthread -lm -ljson-c

SERVER_SRC = server.c list.c survivor.c ai.c map.c view.c
CLIENT_SRC = client.c

all: server client

server: $(SERVER_SRC)
	$(CC) $(CFLAGS) -o server $(SERVER_SRC) $(LDFLAGS)

client: $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o client $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -f server client