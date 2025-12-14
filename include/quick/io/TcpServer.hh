#pragma once

// C Includes
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// C++ Includes
#include <cstdint>
#include <cstring>
#include <expected>

#include "quick/io/Tcp.hh"
#include "quick/utils/Logger.hh"
#include "quick/utils/Timer.hh"

namespace quick::io
{

/// @brief TCP server implementation using POSIX internet sockets.
///        There is no Windows support; currently or ever.
/// @attention Guaranteed to not throw
class TcpServer : public Tcp
{
  private:
    static constexpr std::uint16_t MAX_LISTEN_NUM{10};
    static inline utils::Logger<TcpServer> &m_logger{utils::Logger<TcpServer>::get_instance("TcpServer")};

  public:
    TcpServer() = default;
    explicit TcpServer(const std::string &, std::uint16_t) noexcept;
    explicit TcpServer(std::string &&, std::uint16_t);

    std::expected<void, quick::error::TcpError> start();
    std::expected<SocketRAII, quick::error::TcpError> accept_client();

    __always_inline [[gnu::hot]] auto send(SocketRAII &client, const void *buf, size_t len)
        -> std::expected<std::uint64_t, quick::error::TcpError>
    {
        quick::utils::Timer timer{"TcpServer::send()"};
        ssize_t result = ::send(client, buf, len, MSG_NOSIGNAL);
        if (result < 0) [[unlikely]]
            return std::unexpected(quick::error::TcpError::SEND_FAIL);
        return static_cast<std::uint64_t>(result);
    }

    __always_inline [[gnu::hot]] auto recv(SocketRAII &client, void *buf, size_t len)
        -> std::expected<std::uint64_t, quick::error::TcpError>
    {
        quick::utils::Timer timer{"TcpServer::recv()"};
        ssize_t result = ::recv(client, buf, len, 0);
        if (result < 0) [[unlikely]]
            return std::unexpected(quick::error::TcpError::RECV_FAIL);
        return static_cast<std::uint64_t>(result);
    }
};

// ============================================================================
// Inline Implementation
// ============================================================================

inline TcpServer::TcpServer(const std::string &ip, uint16_t port) noexcept : Tcp{ip, port}
{
}

inline TcpServer::TcpServer(std::string &&ip, uint16_t port) : Tcp{std::move(ip), port}
{
}

inline auto TcpServer::start() -> std::expected<void, quick::error::TcpError>
{
    quick::utils::Timer timer{"TcpServer::start()"};
    LOG_INFO("Attempting to start server on ", _ip, ":", _port);

    if (!create_socket())
    {
        LOG_ERROR("Couldn't create socket.");
        return std::unexpected(quick::error::TcpError::BAD_SOCKET);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    if (::bind(m_sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        LOG_ERROR("Couldn't bind address to socket.");
        return std::unexpected(quick::error::TcpError::BIND_FAIL);
    }

    if (::listen(m_sock, MAX_LISTEN_NUM) < 0)
    {
        LOG_ERROR("Couldn't listen on socket.");
        return std::unexpected(quick::error::TcpError::LISTEN_FAIL);
    }

    _running = true;
    LOG_INFO("Server listening!");
    return {};
}

inline auto TcpServer::accept_client() -> std::expected<SocketRAII, quick::error::TcpError>
{
    quick::utils::Timer timer{"accept_client"};
    int client_fd = ::accept(m_sock, nullptr, nullptr);
    if (client_fd < 0)
        return std::unexpected(quick::error::TcpError::BAD_SOCKET);
    return SocketRAII{client_fd};
}

} // End namespace quick::io