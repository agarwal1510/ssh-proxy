#ifndef CIPHERS_H
#define CIPHERS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <errno.h>

#define BUFFER_SIZE 4096
#define BUFFER 4000

struct ctr_state {
	unsigned char ivec[AES_BLOCK_SIZE];
	unsigned int num;
	unsigned char ecount[AES_BLOCK_SIZE];
};

void client(char *hostname, int port, char *keyfile);
void server(char *hostname, int dport, int lport, char *keyfile);
int encrypt(char *keyfile, char *input, char *output, int inputsize, char *iv);
int decrypt(char *keyfile, char *input, char *output, int inputsize, char *iv);

#endif
