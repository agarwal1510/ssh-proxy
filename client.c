#include "cipher.h"

void client(char *hostname, int port, char *keyfile) {

	struct sockaddr_in serv_addr;
	struct hostent *serverip;
	int socketfd, count, br;
	char plaintext[BUFFER_SIZE], cipher[BUFFER_SIZE], iv[AES_BLOCK_SIZE];
	char *buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	fd_set fdset;
	struct timeval tv;
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket creation error");
		return;
	} 
	if ((serverip = gethostbyname(hostname))==0) {
		fprintf(stderr, "Get hostbyname error client %s", hostname);
		return;
	} 

	bzero(&serv_addr, sizeof(serv_addr));
	//bcopy((char *)serverip->h_addr, (char *)serv_addr.sin_addr.s_addr, serverip->h_length);
	serv_addr.sin_addr.s_addr = ((struct in_addr *)serverip->h_addr)->s_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Client Connection fail\n");
		return;
	}
	//fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
	//fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	//int flag  = fcntl(socketfd, F_GETFL);
	//fcntl(socketfd, F_SETFL, flag | O_NONBLOCK);

	//if (write(socketfd, getIV(), 16) < 0) {
	//	fprintf(stderr, "Error sending IV");
	//	return;
	//}
	while(1) {
		FD_ZERO(&fdset);
		FD_SET(socketfd, &fdset);
		FD_SET(STDIN_FILENO, &fdset);
		tv.tv_sec = 60;
		tv.tv_usec = 0;
		br = 0;
		//fprintf(stdout, "Enter your message:");
		//fflush(stdout);
		//fprintf(stderr, "select called again %d %d", STDIN_FILENO, socketfd);
		int max = socketfd;
		if (STDIN_FILENO > max)
			max = STDIN_FILENO;
		bzero(buffer, BUFFER_SIZE);
		count = select(max+1, &fdset, NULL, NULL, &tv);
		if (count < 0) {
			fprintf(stderr, "Select failed");
			exit(-1);
		} else if (count == 0) {
			fprintf(stderr, "timeout");
			exit(-1);
		} else {
			if (FD_ISSET(STDIN_FILENO, &fdset)) {
				bzero(buffer, BUFFER_SIZE);
				bzero(iv, AES_BLOCK_SIZE);
				bzero(cipher, BUFFER_SIZE);
				br = read(STDIN_FILENO, buffer, BUFFER);
				if (br > 0) {
					//					fprintf(stderr, "S: %d", br);

					if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
						fprintf(stderr, "IV creation error");
						exit(-1);
					}
					memcpy(cipher, iv, AES_BLOCK_SIZE);

					encrypt(keyfile, buffer, cipher, br, iv);
//					fprintf(stderr, "C: %d ", br);
					if (write(socketfd, cipher, br+AES_BLOCK_SIZE) < 0) {
						fprintf(stderr, "write to csocket error");
						exit(-1);
					}
				} else if (br == -1) {
					fprintf(stderr, "E: %s", strerror(errno));
					exit(-1);
				}

			} else if (FD_ISSET(socketfd, &fdset)) {
				bzero(buffer, BUFFER_SIZE);
				bzero(iv, AES_BLOCK_SIZE);
				bzero(plaintext, BUFFER_SIZE);

				br = read(socketfd, buffer, BUFFER+AES_BLOCK_SIZE);
				memcpy(iv, buffer, AES_BLOCK_SIZE);

				decrypt(keyfile, buffer, plaintext, br-AES_BLOCK_SIZE, iv);
				//				fprintf(stderr, "P: %d", br);
				if (write(STDOUT_FILENO,  plaintext, br-AES_BLOCK_SIZE) < 0) {
					fprintf(stderr, "write to ssh -o error");
					exit(-1);
				}

			}

		}




		/*
		   bzero(buffer, BUFFER_SIZE);
		//if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
		if ((count = read(STDIN_FILENO, buffer, 1024)) > 0) {
//fprintf(stderr, "Error reading input\n");
//return;

fprintf(stderr, "Typed: %s", buffer);
encrypt(keyfile, buffer, cipher);
if (write(socketfd, cipher, BUFFER_SIZE) < 0) {
fprintf(stderr, "Client write error\n");
return;
}
}
bzero(buffer, BUFFER_SIZE);
if((count=read(socketfd, buffer, BUFFER_SIZE)) > 0) {
decrypt(keyfile, buffer, plaintext);
fprintf(stderr, "Received: %s", plaintext);
//write(STDOUT_FILENO, buffer, BUFFER_SIZE);
}
		 */
}

}
