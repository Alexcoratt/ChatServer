#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <poll.h>

// TODO: must use poll to serve multiple connections at the same time
int read_msg(int sockfd, int client_sockfd) {
	char buffer[256];
	memset(buffer, '\0', sizeof(buffer));
	int read_res = read(client_sockfd, buffer, sizeof(buffer));
	if (read_res <= 0) {
		fprintf(stderr, "read failed\n");
		return read_res;
	}

	printf("%s\n", buffer);
	return strcmp(buffer, "/stop");
}

void compress_fds_array(struct pollfd * fds, unsigned int * nfds) {
	for (unsigned int i = 0; i < *nfds; ++i) {
		if (fds[i].fd == -1) {
			for(unsigned int j = i; j < *nfds - 1; ++j) {
				fds[j].fd = fds[j + 1].fd;
			}
			--i;
			--*nfds;
		}
	}
}

void close_fds(struct pollfd * fds, unsigned int nfds) {
	for (unsigned int i = 0; i < nfds; ++i)
		if (fds[i].fd >= 0)
			close(fds[i].fd);
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

	struct pollfd fds[32];
	unsigned int nfds = 1;
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	unsigned long timeout = 120000;

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

		unsigned int current_nfds = nfds;
		for (unsigned int i = 0; i < current_nfds; ++i) {
			if (fds[i].revents == 0)
				continue;

			if (fds[i].revents != POLLIN) {
				fprintf(stderr, "Error: fd: %d revents: %d\n", fds[i].fd, fds[i].revents);
				running = 0;
				break;
			}

			if (fds[i].fd == sockfd) {
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
				++nfds;

			} else {
				printf("Descriptor %d is readable\n", fds[i].fd);
				int read_res = read_msg(sockfd, fds[i].fd);
				if (read_res < 0) {
					printf("Connection fd: %d closed\n", fds[i].fd);
					close(fds[i].fd);
					fds[i].fd = -1;
					compress_fds_array(fds, &nfds);
				}
				if (read_res == 0) {
					running = 0;
				}
			}
		}
	}

	//close(sockfd);
	close_fds(fds, nfds);

	return 0;
}
