// Phase 0 server: a single-node, in-memory key-value store served over a
// length-prefixed TCP protocol. One thread per client connection.
//
// In later phases the request handler stops mutating the store directly and
// instead submits commands to the Raft log; the store is only updated once an
// entry is committed and applied. The networking here is reused as-is.

#include "raftkv/kv/store.hpp"
#include "raftkv/net/socket.hpp"

#include <csignal>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using raftkv::kv::KvStore;
using raftkv::net::Socket;

namespace {

// Parse and apply one request line, returning the reply.
// Protocol (text): "SET <key> <value>", "GET <key>", "DEL <key>".
std::string handle_command(KvStore& store, const std::string& request) {
    std::istringstream iss(request);
    std::string op;
    iss >> op;

    if (op == "SET") {
        std::string key, value;
        iss >> key;
        std::getline(iss, value);
        if (!value.empty() && value.front() == ' ') value.erase(0, 1);
        if (key.empty()) return "ERR usage: SET <key> <value>";
        store.set(key, value);
        return "OK";
    }
    if (op == "GET") {
        std::string key;
        iss >> key;
        if (key.empty()) return "ERR usage: GET <key>";
        auto value = store.get(key);
        return value ? ("VALUE " + *value) : "NIL";
    }
    if (op == "DEL") {
        std::string key;
        iss >> key;
        if (key.empty()) return "ERR usage: DEL <key>";
        return store.del(key) ? "OK" : "NIL";
    }
    return "ERR unknown command: " + op;
}

void serve_client(Socket sock, KvStore& store) {
    while (true) {
        auto request = raftkv::net::recv_message(sock);
        if (!request) break;  // client disconnected
        const std::string reply = handle_command(store, *request);
        if (!raftkv::net::send_message(sock, reply)) break;
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::signal(SIGPIPE, SIG_IGN);  // never die because a client vanished mid-write

    uint16_t port = 5000;
    if (argc >= 2) port = static_cast<uint16_t>(std::stoi(argv[1]));

    Socket listener = raftkv::net::listen_on(port);
    std::cout << "raftkv node listening on port " << port << std::endl;

    KvStore store;
    while (true) {
        Socket client = raftkv::net::accept_on(listener);
        if (!client.valid()) continue;
        std::thread(serve_client, std::move(client), std::ref(store)).detach();
    }
    return 0;
}
