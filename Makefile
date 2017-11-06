CC=gcc
CFLAGS=-O3

all:
	$(CC) $(CFLAGS) -g cipher.c client.c server.c hw3.c -lpthread -lcrypto -o pbproxy

clean:
	$(RM) pbproxy

