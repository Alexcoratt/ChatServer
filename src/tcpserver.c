#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int loop(int sockfd, int client_sockfd) {
	char buffer[256];
	do {
		memset(buffer, '\0', sizeof(buffer));
		if (read(client_sockfd, buffer, sizeof(buffer)) <= 0) {
			fprintf(stderr, "read failed\n");
			return -7;
		}

		printf("%s\n", buffer);
	} while (strcmp(buffer, "/stop"));

	return 0;
}

int main(int argc, char ** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "socket failed\n");
		return -2;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		fprintf(stderr, "bind failed\n");
		close(sockfd);
		return -4;
	}

	if (listen(sockfd, 3) == -1) {
		fprintf(stderr, "listen failed\n");
		close(sockfd);
		return -5;
	}



	int res = 0;
	do {
		struct sockaddr_in client;
		unsigned int clientaddrlen = sizeof(client);
		int client_sockfd = accept(sockfd, (struct sockaddr *)&client, &clientaddrlen);
		if (client_sockfd < 0) {
			fprintf(stderr, "accept failed\n");
			break;
		}
		printf("Server got connection from %s\n", inet_ntoa(client.sin_addr));

		res = loop(sockfd, client_sockfd);
		close(client_sockfd);
	} while (res != 0);

	close(sockfd);

	return res;
}
