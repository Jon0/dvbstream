#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "socket.h"

constexpr int buf_size = 4 * 1024;

int sendall(int outfd, char *buffer, int buf_size) {
	while (buf_size > 0) {
		int w = write(outfd, buffer, buf_size);
		if (w > 0) {
			buf_size -= w;
			buffer += w;
		}
		else {
			os::error("ERROR writing fd");
		}

		// note incomplete writes
		if (buf_size > 0) {
			std::cout << "incomplete write: " << buf_size << "\n";
		}
	}
	return buf_size;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		return 0;
	}

	// wait for connection
	os::tcp_acceptor acceptor(8555);
	os::tcp_socket sock(acceptor);
	//os::fdbuf buf(sock.fd());
	//std::iostream tcp_stream(&buf);

	std::string path = argv[1];
	std::cout << "opening " << path << "\n";
	os::location loc(path);
	int srcfd = loc.openfd();
	if (srcfd < 0) {
		os::error("ERROR opening file");
	}
	char buffer[buf_size];

	// transfer rate
	int transferred = 0;
	auto start_time = std::chrono::system_clock::now();
	while (true) {
		int b = read(srcfd, buffer, buf_size);
		if (b < 0) {
			os::error("ERROR reading fd");
		}
		sendall(sock.fd(), buffer, b);

		// record transfer rate
		transferred += b;
		if (transferred > (1 << 16)) {
			auto end_time = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end_time - start_time;
			auto rate = static_cast<double>(transferred) / (elapsed_seconds.count() * 1024 * 1024);
			transferred = 0;
			start_time = end_time;
			std::cout << "transferring " << rate << " Mbps\n";
		}
	}

}
