#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <string>

#include "fiah/Error.hh"
#include "fiah/io/SocketRAII.hpp"

namespace fiah::io
{

class Tcp
{
  protected:
    std::string _ip{};
    std::uint16_t _port{};
    SocketRAII m_sock{-1};
    bool _running{false};

    Tcp() = default;
    explicit Tcp(const std::string &ip, std::uint16_t port) : _ip{ip}, _port{port}
    {
        static_assert(!std::is_copy_constructible_v<Tcp>);
        static_assert(!std::is_copy_assignable<Tcp>::value);
        static_assert(std::is_move_constructible_v<Tcp>);
        static_assert(std::is_move_assignable_v<Tcp>);
    }

    explicit Tcp(std::string &&ip, std::uint16_t port) : _ip{std::move(ip)}, _port{port}
    {
        static_assert(!std::is_copy_constructible_v<Tcp>);
        static_assert(!std::is_copy_assignable<Tcp>::value);
        static_assert(std::is_move_constructible_v<Tcp>);
        static_assert(std::is_move_assignable_v<Tcp>);
    }

    auto create_socket() -> std::expected<void, TcpError>
    {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            return std::unexpected(TcpError::BAD_SOCKET);
        m_sock = fd;
        return {};
    }

    int get_fd() const noexcept
    {
        return static_cast<int>(m_sock);
    }

    [[gnu::hot]] ssize_t send_data(const void *buf, size_t len, int flags = 0)
    {
        return ::send(m_sock, buf, len, flags);
    }

    [[nodiscard]] [[gnu::hot]] ssize_t recv_data(void *buf, size_t len, int flags = 0)
    {
        return ::recv(m_sock, buf, len, flags);
    }

  public:
    Tcp(const Tcp &) = delete;
    Tcp &operator=(const Tcp &) = delete;

    Tcp(Tcp &&) = default;
    Tcp &operator=(Tcp &&) = default;

    virtual ~Tcp() noexcept = default;
};
} // End namespace fiah::io