# pragma once 

# include <cstdint> 
# include <optional> 
# include <string> 

namespace raftkv::net {

// RALI owner of a socket file descriptor. Movable, non copyable.
class Socket {
public:
    Socket() = default; 
    explicit Socket(int fd) : fd_(fd) {}
    ~Socket();

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    int fd() const { return fd_; }
    bool valid() const { return fd_ >= 0; }
    void close();

private:
    int fd_ = -1;

};

// Create a listening socket bound to `port` on all interfaces.
Socket listen_on(uint16_t port);
 
// Accept one connection. Blocks. Returns an invalid Socket on error.
Socket accept_on(const Socket& listener);
 
// Connect to host:port. Returns an invalid Socket on failure.
Socket connect_to(const std::string& host, uint16_t port);
 
// Length-prefixed framed messaging. These correctly handle partial reads/writes.
bool send_message(const Socket& sock, const std::string& payload);
std::optional<std::string> recv_message(const Socket& sock);
 
}  // namespace raftkv::net


