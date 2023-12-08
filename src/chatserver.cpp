#include <iostream>
#include <map>
#include <string>
#include <stdexcept>

#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

class ChatUser {
private:
	std::string _name;

public:
	ChatUser(std::string const & name) : _name{name} {}
	std::string getName() const { return _name; }
};

int openSocket(unsigned int port, unsigned short connLimit, int domain, int sock_type, int protocol = 0) {
	int sockfd = socket(domain, sock_type, protocol);
	if (sockfd < 0)
		throw std::runtime_error("socket failed");

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
		throw std::runtime_error("bind failed");
	if (listen(sockfd, connLimit) != 0)
		throw std::runtime_error("listen failed");

	return sockfd;
}

void loop(int serverSocket, int clientSocket) {
	char buffer[256];
	do {
		memset(buffer, 0, strlen(buffer));
		if (recv(clientSocket, buffer, sizeof(buffer), 0) == -1)
			throw std::runtime_error("receive failed");

		std::cout << buffer << std::endl;

		if (send(clientSocket, buffer, sizeof(buffer), 0) < 0)
			throw std::runtime_error("send failed");

	} while (strcmp(buffer, "/stop"));
}

int main(int argc, char ** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[1]);
		fflush(stderr);
		return 1;
	}

	//std::map<int, ChatUser> users;
	int sockfd;

	try {
		sockfd = openSocket(atoi(argv[1]), 10, AF_INET, SOCK_STREAM);

		int clientSocket = accept(sockfd, nullptr, nullptr);
		if (clientSocket == -1)
			throw std::runtime_error("accept failed");
		loop(sockfd, clientSocket);
	} catch (std::exception const & err) {
		std::cerr << err.what() << std::endl;
	}

	if (sockfd >= 0)
		close(sockfd);

	return 0;
}
