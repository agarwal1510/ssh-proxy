#include "cipher.h"
#include <stdio.h>
#include <string.h>

FILE *readfile;
FILE *writefile;

AES_KEY key;
int bytes_read, bytes_written;
unsigned char indata[AES_BLOCK_SIZE];
unsigned char outdata[AES_BLOCK_SIZE];
unsigned char iv[AES_BLOCK_SIZE];
struct ctr_state state;

int init_ctr(struct ctr_state *state, const unsigned char iv[16])
{
    /* aes_ctr128_encrypt requires 'num' and 'ecount' set to zero on the
    * first call. */
    state->num = 0;
    memset(state->ecount, 0, AES_BLOCK_SIZE);
 
    /* Initialise counter in 'ivec' to 0 */
    memset(state->ivec + 8, 0, 8);
 
    /* Copy IV into 'ivec' */
    memcpy(state->ivec, iv, 8);
}

char *getIV() {
	FILE *iv_file = fopen("IV.txt", "r");
        if (iv_file == NULL) {
                fprintf(stderr, "Error reading IV");
                exit(-1);
        }
        if (fread(iv, 1, 16, iv_file) < 16) {
                fprintf(stderr, "IV smaller");
        }
        fclose(iv_file);
	return iv;
}

void generateIV() {

        if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
                fprintf(stderr, "IV creation error");
                exit(-1);
        }
        FILE *iv_file = fopen("IV.txt", "w");
        if (iv_file == NULL) {
                fprintf(stderr, "Error opening IV");
                exit(-1);
        }
        fwrite(iv, 1, sizeof(iv), iv_file);
	//fprintf(stderr, "%d", sizeof(iv));
        fclose(iv_file);
	
}

int encrypt(char* keyfile, char *input, char *output) {
	unsigned char *enc_key = (char *)malloc(sizeof(char)*16);
	int bytes_encrypted = 0, bytes_read = 0;
	int inputsize = strlen(input);

	generateIV();
	FILE *key_file = fopen(keyfile, "r");
	if (key_file == NULL){
		fprintf(stderr, "Cannot open key file");
		exit(-1);
	}
	if (fread(enc_key, 1, 16, key_file) < 16) {
		fprintf(stderr, "Key size smaller");
		exit(-1);
	}
	fclose(key_file);
	
	if (AES_set_encrypt_key(enc_key, 128, &key) < 0) {
		fprintf(stderr, "Enryption key error");
		exit(-1);
	}
	init_ctr(&state, iv);
	bytes_read = 0;
	while(bytes_encrypted != inputsize) {
		
		for (int j = bytes_encrypted; j < inputsize && j < (AES_BLOCK_SIZE+bytes_encrypted); j++) {
			indata[bytes_read++] = input[j];
		}
		//printf("\nBE: %d %d", bytes_read, bytes_encrypted);
		AES_ctr128_encrypt(indata, outdata, bytes_read, &key, state.ivec, state.ecount, &(state.num));
		
		for (int i = 0; i < bytes_read; i++) {
			output[i+bytes_encrypted] = outdata[i];
		}
		bytes_encrypted += bytes_read;
		if (bytes_read < AES_BLOCK_SIZE) {
			//fprintf(stdout, "Cipher text: %s", output);
			break;
		}
		bytes_read = 0;
	}
	output[bytes_encrypted] = '\0';
	//printf("\n%d %d %s", inputsize, bytes_encrypted, output);
}

int decrypt(char* keyfile, char *input, char *output) {
        unsigned char *enc_key = (char *)malloc(sizeof(char)*16);
        int bytes_encrypted = 0, bytes_read = 0;
        int inputsize = strlen(input);
	int count;
	
	//fprintf(stderr, "%s", input);
	FILE *iv_file = fopen("IV.txt", "r");
        if (iv_file == NULL) {
		fprintf(stderr, "Error reading IV");
		exit(-1);
	}
	if ((count = fread(iv, 1, 16, iv_file)) < 16) {
		fprintf(stderr, "IV smaller %d", sizeof(iv));
	}
	fclose(iv_file);
        FILE *key_file = fopen(keyfile, "r");
        if (key_file == NULL){
                fprintf(stderr, "Cannot open key file");
                exit(-1);
        }
        if (fread(enc_key, 1, 16, key_file) < 16) {
                fprintf(stderr, "Key size smaller");
                exit(-1);
        }
        fclose(key_file);

        if (AES_set_encrypt_key(enc_key, 128, &key) < 0) {
                fprintf(stderr, "Enryption key error");
                exit(-1);
        }
        init_ctr(&state, iv);
	bytes_read = 0;
        while(bytes_encrypted != inputsize) {

                for (int j = bytes_encrypted; j < (AES_BLOCK_SIZE+bytes_encrypted) && j < inputsize; j++) {
                        indata[bytes_read++] = input[j];
                }
		//printf("\n%d", bytes_read);
                AES_ctr128_encrypt(indata, outdata, bytes_read, &key, state.ivec, state.ecount, &(state.num));
         
                for (int i = 0; i < bytes_read; i++) {
                        output[i+bytes_encrypted] = outdata[i];
                }
                bytes_encrypted += bytes_read;

                if (bytes_read < AES_BLOCK_SIZE) {
                        //fprintf(stdout, "Plain text: %s", output);
                        break;
                }
                bytes_read = 0;
        }
	output[bytes_encrypted] = '\0';
}
/*
int main() {
	char *input = "Hello this is plain text and it was successfully encrypted and decrypted";
	char *output1 = (char*) malloc(sizeof(char)*strlen(input)+1);
	char *output2 = (char*) malloc(sizeof(char)*strlen(input)+1);
	char *keyfile = "key.txt";
	encrypt(keyfile, input,output1);
	printf("\nOutput1: %s", output1);
	decrypt(keyfile, output1, output2);
	printf("\nOutput2: %d %s", AES_BLOCK_SIZE, output2);
	return 1;
} */

