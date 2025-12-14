#include <arpa/inet.h>
#include <expected>

#include "fiah/io/TcpClient.hh"
#include "fiah/io/TcpError.hh"
#include "fiah/structs/Structs.hh"

namespace fiah::io
{

TcpClient::TcpClient(const std::string &ip, const uint16_t port) : Tcp{ip, port}
{
}

TcpClient::TcpClient(std::string &&ip, const uint16_t port) : Tcp{std::move(ip), port}
{
}

auto TcpClient::connect_to_server() -> std::expected<void, TcpError>
{
    if (!create_socket())
        return std::unexpected(TcpError::BAD_SOCKET);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = ::htons(_port);
    if (::inet_pton(AF_INET, _ip.c_str(), &server.sin_addr) <= 0)
        return std::unexpected(TcpError::INVALID_IP);

    if (::connect(m_sock, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0)
        return std::unexpected(TcpError::CONNECT_FAIL);
    return {};
}

[[gnu::hot]] auto TcpClient::send(const void *buf, size_t len) -> std::expected<std::uint64_t, TcpError>
{
    ssize_t result = Tcp::send_data(buf, len);
    if (result < 0) [[unlikely]]
        return std::unexpected(TcpError::SEND_FAIL);
    return static_cast<std::uint64_t>(result);
}

[[gnu::hot]] auto TcpClient::recv(void *buf, size_t len) -> std::expected<std::uint64_t, TcpError>
{
    ssize_t result = Tcp::recv_data(buf, len);
    if (result < 0) [[unlikely]]
        return std::unexpected(TcpError::RECV_FAIL);
    return static_cast<std::uint64_t>(result);
}
} // namespace fiah::io