#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

int loop(struct sockaddr_in * server) {
	char buffer[256] = "";

	do {
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			fprintf(stderr, "socket failed\n");
			return -2;
		}

		if (connect(sockfd, (struct sockaddr *)server, sizeof(*server)) == -1) {
			fprintf(stderr, "connect failed\n");
			return -4;
		}

		memset(buffer, '\0', sizeof(buffer));
		printf("Message: ");
		scanf("%s", buffer);

		if (write(sockfd, buffer, sizeof(buffer)) == -1) {
			fprintf(stderr, "write failed\n");
			return -5;
		}

		close(sockfd);
	} while (strcmp(buffer, "/exit"));

	return 0;
}

int main(int argc, char ** argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		return -1;
	}

	struct hostent * hostname = gethostbyname(argv[1]);
	if (hostname == NULL) {
		fprintf(stderr, "gethostbyname failed\n");
		return -3;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = *(unsigned long *)hostname->h_addr;

	int res = loop(&server);
	printf("Finished\n");

	return res;
}
