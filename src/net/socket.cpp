#include "raftkv/net/socket.hpp" 

#include <arpa/inet.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 

#include <cerrno> 
#include <cstring> 
#include <stdexcept> 

namespace raftkv::net { 

Socket::~Socket() { close(); } 

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; } 

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close(); 
        fd_ = other.fd_;
        other.fd_ =-1; 
    }
    return *this; 
}

void Socket::close() { 
    if (fd_ >= 0) {
        ::close(fd_);
        fd_=-1;
    }
}

Socket listen_on(uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0); 
    if (fd <0) throw std::runtime_error(std::string("socket: ") + std::strerror(errno)); 

int yes = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
 
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
 
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        throw std::runtime_error(std::string("bind: ") + std::strerror(errno));
    }
    if (::listen(fd, SOMAXCONN) < 0) {
        ::close(fd);
        throw std::runtime_error(std::string("listen: ") + std::strerror(errno));
    }
    return Socket(fd);
}
 
Socket accept_on(const Socket& listener) {
    int fd = ::accept(listener.fd(), nullptr, nullptr);
    return Socket(fd);  // invalid (fd < 0) on error; caller checks valid()
}
 
Socket connect_to(const std::string& host, uint16_t port) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
 
    addrinfo* res = nullptr;
    const std::string port_str = std::to_string(port);
    if (::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        return Socket();
    }
 
    int fd = -1;
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (::connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
        ::close(fd);
        fd = -1;
    }
    ::freeaddrinfo(res);
    return Socket(fd);
}
 
namespace {
 
bool write_all(int fd, const char* buf, size_t n) {
    size_t sent = 0;
    while (sent < n) {
        ssize_t r = ::send(fd, buf + sent, n - sent, MSG_NOSIGNAL);
        if (r < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (r == 0) return false;
        sent += static_cast<size_t>(r);
    }
    return true;
}
 
bool read_all(int fd, char* buf, size_t n) {
    size_t got = 0;
    while (got < n) {
        ssize_t r = ::recv(fd, buf + got, n - got, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (r == 0) return false;  // peer closed the connection
        got += static_cast<size_t>(r);
    }
    return true;
}
 
}  // namespace
 
bool send_message(const Socket& sock, const std::string& payload) {
    uint32_t len = htonl(static_cast<uint32_t>(payload.size()));
    if (!write_all(sock.fd(), reinterpret_cast<const char*>(&len), sizeof(len))) return false;
    return write_all(sock.fd(), payload.data(), payload.size());
}
 
std::optional<std::string> recv_message(const Socket& sock) {
    uint32_t len_be = 0;
    if (!read_all(sock.fd(), reinterpret_cast<char*>(&len_be), sizeof(len_be))) return std::nullopt;
    const uint32_t len = ntohl(len_be);
    std::string payload(len, '\0');
    if (len > 0 && !read_all(sock.fd(), payload.data(), len)) return std::nullopt;
    return payload;
}
 
}  // namespace raftkv::net