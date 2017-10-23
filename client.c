#include "cipher.h"

void client(char *hostname, int port, char *keyfile) {

	struct sockaddr_in serv_addr;
	struct hostent *serverip;
	int socketfd, count;
	char plaintext[BUFFER_SIZE], cipher[BUFFER_SIZE];
	char *buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket creation error");
		return;
	} 
	if ((serverip = gethostbyname(hostname))==0) {
		fprintf(stderr, "Get hostbyname error");
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
	fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	int flag  = fcntl(socketfd, F_GETFL);
	fcntl(socketfd, F_SETFL, flag | O_NONBLOCK);

	//if (write(socketfd, getIV(), 16) < 0) {
	//	fprintf(stderr, "Error sending IV");
	//	return;
	//}
	while(1) {
		//fprintf(stdout, "Enter your message:");
		//fflush(stdout);
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

	}

	}
