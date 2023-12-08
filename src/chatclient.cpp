#include <iostream>
#include <string>
#include <stdexcept>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

void loop(int sockfd, sockaddr_in & server) {
	if (connect(sockfd, (sockaddr *)&server, sizeof(server)) < 0)
		throw std::runtime_error("connect failed");

	std::string message;
	std::cout << "message: ";
	std::getline(std::cin, message);

	while (message != "/exit") {
		char cstrmessage[message.size()];
		strcpy(cstrmessage, message.c_str());

		if (send(sockfd, cstrmessage, sizeof(cstrmessage), 0) < 0)
			throw std::runtime_error("send failed");

		char response[256];
		if (recv(sockfd, response, sizeof(response), 0) < 0)
			throw std::runtime_error("receive failed");

		std::cout << "response: " << response << std::endl;

		std::cout << "message: ";
		std::getline(std::cin, message);
	}
}

int main(int argc, char ** argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		fflush(stderr);
		return 1;
	}

	hostent * hostname = gethostbyname(argv[1]);
	if (hostname == (struct hostent *) 0) {
		std::cerr << "gethostbyname failed" << std::endl;
		return 2;
	}

	unsigned int port = atoi(argv[2]);
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = *(unsigned long *)hostname->h_addr;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "socket failed" << std::endl;
		return 3;
	}

	try {
		loop(sockfd, server);
	} catch (std::exception const & err) {
		std::cerr << err.what() << std::endl;
		return -1;
	}

	close(sockfd);
	std::cout << "Success" << std::endl;

	return 0;
}
