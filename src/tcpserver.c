#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <poll.h>

#define USER_COUNT 32

void compress_fds_array(struct pollfd * fds, char ** names, unsigned int * nfds) {
	for (unsigned int i = 0; i < *nfds; ++i) {
		if (fds[i].fd == -1) {
			for(unsigned int j = i; j < *nfds - 1; ++j) {
				fds[j].fd = fds[j + 1].fd;
				names[j] = names[j + 1];
			}
			--i;
			--*nfds;
		}
	}
}

void close_fds(struct pollfd * fds, char ** names, unsigned int nfds) {
	for (unsigned int i = 0; i < nfds; ++i)
		if (fds[i].fd >= 0) {
			close(fds[i].fd);
			free(names[i]);
		}
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

	if (listen(sockfd, USER_COUNT - 1) == -1) {
		fprintf(stderr, "listen failed\n");
		close(sockfd);
		return -5;
	}

	struct pollfd fds[USER_COUNT];
	char * names[USER_COUNT];
	unsigned int nfds = 1;
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	unsigned long timeout = 120000;
	names[0] = malloc(7);
	strcpy(names[0], "server");

	int running = 1;
	while (running) {
		printf("poll started\n");
		int poll_res = poll(fds, nfds, timeout);

		if (poll_res < 0) {
			fprintf(stderr, "poll failed\n");
			break;
		}

		if (poll_res == 0) {
			printf("poll timed out\n");
			break;
		}

		for (unsigned int i = 1; i < nfds; ++i) {
			if (fds[i].revents == 0)
				continue;

			if (fds[i].revents != POLLIN) {
				fprintf(stderr, "Error: fd: %d revents: %d\n", fds[i].fd, fds[i].revents);
				running = 0;
				break;
			}

			printf("Descriptor %d is readable\nnfds: %d\n", fds[i].fd, nfds);

			char buffer[256];
			memset(buffer, '\0', sizeof(buffer));
			if (read(fds[i].fd, buffer, sizeof(buffer)) <= 0) {
				printf("Connection fd: %d closed\n", fds[i].fd);
				close(fds[i].fd);
				fds[i].fd = -1;
				free(names[i]);
				compress_fds_array(fds, names, &nfds);
			}

			if (strlen(buffer)) {
				char msg[256];
				memset(msg, '\0', sizeof(msg));
				strcpy(msg, names[i]);
				strcat(msg, ": ");
				strcat(msg, buffer);

				for (unsigned int j = 1; j < nfds; ++j) {
					if (j != i)
						write(fds[j].fd, msg, sizeof(msg));
				}
				printf("%s\n", msg);

				// performing commands
				if (buffer[0] == '/') {
					if (strcmp(buffer, "/stop") == 0)
						running = 0;
				}
			}
		}

		if (fds[0].revents == POLLIN) {
			printf("sockfd readable\n");
			struct sockaddr_in client;
			unsigned int clientaddrlen = sizeof(client);
			int client_sockfd = accept(sockfd, (struct sockaddr *)&client, &clientaddrlen);
			if (client_sockfd < 0) {
				fprintf(stderr, "accept failed\n");
				break;
			}
			printf("Server got connection from %s; fd: %d\n", inet_ntoa(client.sin_addr), client_sockfd);

			fds[nfds].fd = client_sockfd;
			fds[nfds].events = POLLIN;

			char new_name[8] = "user";
			char user_num[3];
			sprintf(user_num, "%d", nfds);
			strcat(new_name, user_num);

			names[nfds] = malloc(16);
			strcpy(names[nfds], new_name);

			++nfds;
		}
	}

	//close(sockfd);
	close_fds(fds, names, nfds);

	return 0;
}
