// Minimal CLI client: sends one command and prints the reply.
//   raftkv_client <host> <port> <command...>
//   raftkv_client 127.0.0.1 5000 SET foo bar

#include "raftkv/net/socket.hpp"

#include <cstdint>
#include <iostream>
#include <string>

using raftkv::net::Socket;

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: " << argv[0] << " <host> <port> <command...>\n"
                  << "  e.g. " << argv[0] << " 127.0.0.1 5000 SET foo bar\n";
        return 1;
    }

    const std::string host = argv[1];
    const uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    std::string command;
    for (int i = 3; i < argc; ++i) {
        if (i > 3) command += ' ';
        command += argv[i];
    }

    Socket sock = raftkv::net::connect_to(host, port);
    if (!sock.valid()) {
        std::cerr << "error: could not connect to " << host << ":" << port << "\n";
        return 1;
    }
    if (!raftkv::net::send_message(sock, command)) {
        std::cerr << "error: failed to send request\n";
        return 1;
    }
    auto reply = raftkv::net::recv_message(sock);
    if (!reply) {
        std::cerr << "error: no reply from server\n";
        return 1;
    }
    std::cout << *reply << "\n";
    return 0;
}
