// C Includes
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// C++ Includes
#include <cstring>
#include <expected>
#include <iostream>
#include <thread>

// FastInAHurry Includes
#include "fiah/io/TcpError.hh"
#include "fiah/io/TcpServer.hh"
#include "fiah/utils/Logger.hh"
#include "fiah/utils/Timer.hpp"

namespace fiah::io
{

TcpServer::TcpServer(const std::string &ip, uint16_t port) noexcept : Tcp{ip, port}
{
}

TcpServer::TcpServer(std::string &&ip, uint16_t port) noexcept(noexcept(std::string{std::move("Hi mom!")}))
    : Tcp{std::move(ip), port}
{
}

auto TcpServer::start() -> std::expected<void, TcpError>
{
    utils::Timer timer{"TcpServer::start()"};
    LOG_INFO("Attempting to start server on ", _ip, ":", _port);

    if (!create_socket())
    {
        LOG_ERROR("Couldn't create socket.");
        return std::unexpected(TcpError::BAD_SOCKET);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    if (::bind(m_sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        LOG_ERROR("Couldn't bind "
                  "address to socket.");
        return std::unexpected(TcpError::BIND_FAIL);
    }

    if (::listen(m_sock, MAX_LISTEN_NUM) < 0)
    {
        LOG_ERROR("Couldn't listen on socket.");
        return std::unexpected(TcpError::LISTEN_FAIL);
    }

    _running = true;
    LOG_INFO("Server listening!");
    return {};
}

auto TcpServer::accept_client() -> std::expected<SocketRAII, TcpError>
{
    utils::Timer timer{"accept_client"};
    int client_fd = ::accept(m_sock, nullptr, nullptr);
    if (client_fd < 0)
        return std::unexpected(TcpError::BAD_SOCKET);
    return SocketRAII{client_fd};
}

} // namespace fiah::io