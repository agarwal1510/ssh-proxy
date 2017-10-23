#include "cipher.h"

int main(int argc, char **argv) {


	unsigned int lport;
	int dport, param, sflag = 0;
	char *keyfile = NULL, *hostname = NULL;	
	printf("In main\n");
	if (argc > 1) {
		for (int j =1; j < argc; j=j+2) {
			if (strcmp(argv[j], "-l") == 0){
				lport = atoi(argv[j+1]);
				sflag = 1;
				param += 2;
				printf("lport found %d \n", lport);
			} else if (strcmp(argv[j], "-k") == 0) {
				param += 2;
				keyfile = argv[j+1];
				printf("filename: %s\n",keyfile);
			} else {
				param += 2;
				hostname = argv[j];
				dport = atoi(argv[j+1]);
				printf("host:dport  %s:%d\n",hostname, dport);
			}
		}
		/*if (param < argc) {
			while (argc != param) {
				strcat(expr, argv[param]);
				strcat(expr, " ");
				param += 1;
			}
			//printf("Expression: %s\n",expr);
		}*/
	}
	if (sflag) {
		printf("Server called\n");
		server(hostname, dport, lport, keyfile);
	} else {
		fprintf(stdout, "Client called\n");
		client(hostname, dport, keyfile);
	}
	return 1;
}
