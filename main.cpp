#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "socket.h"

constexpr int buf_size = 1024 * 1024;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		return 0;
	}

	// wait for connection
	os::tcp_acceptor acceptor(8555);
	os::tcp_socket sock(acceptor);
	os::fdbuf buf(sock.fd());
	std::iostream tcp_stream(&buf);

	std::string path = argv[1];
	std::cout << "opening " << path << "\n";

	os::location loc(path);
	int srcfd = loc.openfd();
	char buffer[buf_size];

	// transfer rate
	int transferred = 0;
	auto start_time = std::chrono::system_clock::now();
	while (tcp_stream.good()) {
		auto b = read(srcfd, buffer, buf_size);
		tcp_stream.write(buffer, b);
		transferred += b;
		if (transferred > (1 << 20)) {
			auto end_time = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end_time - start_time;
			auto rate = static_cast<double>(transferred) / (elapsed_seconds.count() * 1024 * 1024);
			transferred = 0;
			start_time = end_time;
			std::cout << "transferring " << rate << " Mbps\n";
		}
	}

}
