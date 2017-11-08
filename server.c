#include "cipher.h"

int des_port;
char *host_name;


struct client_info {
	int csocket;
	char *keyfile;
};

void *thread_handler(void *thread) {
	int BUFFER = 4000;
	int sshfd, count, brecv = 0;
	char buffer[BUFFER];
	char plaintext[BUFFER], cipher[BUFFER_SIZE], iv[AES_BLOCK_SIZE];
	struct sockaddr_in ssh_addr;
	struct hostent *host;
	struct client_info *info = (struct client_info *)thread;
	fd_set fdset;
	struct timeval tv;

	if ((sshfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Sshd socket error");
		exit(-1);
	}
	//sshfd = info->csocket;
	//printf("sshfd %d %d %s", sshfd, des_port, host_name);
	if ((host = gethostbyname(host_name)) == 0) {
		fprintf(stderr, "Get hostbyname error");
		exit(-1);
	}
	bzero(&ssh_addr, sizeof(ssh_addr));
	ssh_addr.sin_family = AF_INET;
	ssh_addr.sin_addr.s_addr = ((struct in_addr *)host->h_addr)->s_addr;
	ssh_addr.sin_port = htons(des_port);

	if (connect(sshfd, (struct sockaddr *)&ssh_addr, sizeof(ssh_addr)) < 0){
		fprintf(stderr, "sshd connection falied");
		exit(-1);
	}

	//fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	//fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
	//int flag = fcntl(sshfd, F_GETFL);
	//if (fcntl(sshfd, F_SETFL, flag | O_NONBLOCK) < 0) {
	//	fprintf(stderr, "fcntl error");
	//}

	while (1) {
		FD_ZERO(&fdset);
		FD_SET(sshfd, &fdset);
		FD_SET(info->csocket, &fdset);
		tv.tv_sec = 60;
		tv.tv_usec = 0;
		brecv = 0;
		//bzero(buffer, BUFFER_SIZE);
		//fprintf(stderr, "Read from csocket");
		//if((count = read(info->csocket, buffer, BUFFER_SIZE)) > 0) {
		//	fprintf(stdout, "\nMessage received:");
		//	decrypt(info->keyfile, buffer, plaintext);
		//	fprintf(stdout, "%s\n", plaintext);
		//write(STDOUT_FILENO, buffer, BUFFER_SIZE);


		//	if ((count = write(sshfd, plaintext, strlen(plaintext)+1)) < 0) {
		//		fprintf(stderr, "Ssh fd write error");
		//		exit(-1);
		//		}
		//	fprintf(stderr, "Count: %d %d", count, strlen(plaintext));
		//}
		//fprintf(stderr, "Read from ssh");
		//fprintf(stderr, "select called again %d %d", sshfd, info->csocket);
		int max = sshfd;
		if (info->csocket > max) 
			max = info->csocket;
		bzero(buffer, BUFFER);
		count = select(max+1, &fdset, NULL, NULL, &tv);
		if (count < 0) {
			fprintf(stderr, "Select failed");
			exit(-1);
		} else if (count == 0) {
			fprintf(stderr, "timeout");
			exit(-1);
		} else {
			if (FD_ISSET(sshfd, &fdset)) {
				bzero(buffer, BUFFER);
				bzero(cipher, BUFFER_SIZE);
				bzero(iv, AES_BLOCK_SIZE);

				usleep(1000);
				brecv = read(sshfd, buffer, BUFFER);
				if (brecv > 0) {
					//	fprintf(stderr, "R: %d", brecv);
					if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
						fprintf(stderr, "IV creation error");
						exit(-1);
					}
					memcpy(cipher, iv, AES_BLOCK_SIZE);

					encrypt(info->keyfile, buffer, cipher, brecv, iv);
					//fprintf(stderr, "C: %d %d", sizeof(cipher), brecv);
					if (write(info->csocket, cipher, brecv+AES_BLOCK_SIZE) < 0) {
						fprintf(stderr, "write to csocket error");
						exit(-1);
					}
				} else if (brecv == -1) {
					fprintf(stderr, "E: %s", strerror(errno));
					exit(-1);
				}

			} else if (FD_ISSET(info->csocket, &fdset)) {
				bzero(buffer, BUFFER);
				bzero(plaintext, BUFFER);
				bzero(iv, AES_BLOCK_SIZE);

				brecv = read(info->csocket, buffer, BUFFER);
				if (brecv > 0) {
					//	fprintf(stderr, "S: %d", brecv);
					memcpy(iv, buffer, AES_BLOCK_SIZE);
					decrypt(info->keyfile, buffer, plaintext, brecv-AES_BLOCK_SIZE, iv);
					//fprintf(stderr, "P: %d %d", sizeof(plaintext), brecv);

					if (write(sshfd, plaintext, brecv-AES_BLOCK_SIZE) < 0) {
						fprintf(stderr, "Ssh fd write error");
						exit(-1);
					}
				}

			}

		}
		//if((count = read(sshfd, buffer, BUFFER_SIZE)) > 0) {
		//fprintf(stderr, "From ssh: %s", buffer);
		//encrypt(info->keyfile, buffer, cipher);
		//fprintf(stderr, "Cipher: %s", cipher);
		//if (write(info->csocket, cipher, BUFFER_SIZE) < 0) {
		//	fprintf(stderr, "write to csocket error");
		//	exit(-1);
		//}
		//}
	}

}

void server(char* hostname, int dport, int lport, char *keyfile) {
	struct sockaddr_in server, client;
	int serverfd, csocket;
	char buffer[BUFFER_SIZE];
	int rc, on =1;
	des_port = dport;
	host_name = hostname;
	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverfd == -1) {
		fprintf(stderr, "Socket creation error");
		return;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(lport);

	if (bind(serverfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		fprintf(stderr, "Binding to socket failed");
		return;
	}

	listen(serverfd, 10);
	int clen = sizeof(struct sockaddr_in);
	while (1) {
		csocket = accept(serverfd, (struct sockaddr *)&client, (socklen_t *)&clen);
		//printf("\nConnection accepted %d\n", csocket);
		struct client_info *info = (struct client_info *)malloc(sizeof(struct client_info));
		info->csocket = csocket;
		info->keyfile = keyfile;

		/*bzero(buffer, sizeof(buffer));
		  if (read(info->csocket, buffer, BUFFER_SIZE) > 0) {
		  fprintf(stdout, "\nMessage received:");
		  fprintf(stdout, "%s", buffer);
		  }

		  if (write(info->csocket, buffer, strlen(buffer)) < 0) {
		  fprintf(stderr, "Server write error");
		  exit(-1);
		  }*/
		pthread_t cthread;

		if (pthread_create(&cthread, NULL, thread_handler, (void *) info) < 0) {
			fprintf(stderr, "Thread creation error");
			return;
		}

	}
	close(serverfd);

}

