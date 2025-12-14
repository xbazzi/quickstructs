#pragma once

#include <cstdint>
#include <expected>

#include "fiah/io/Tcp.hh"
#include "fiah/io/TcpError.hh"

namespace fiah::io {

class TcpClient : public Tcp
{
public:
    using Tcp::send_data;
    using Tcp::recv_data;

    TcpClient() = default;
    explicit TcpClient(const std::string&, const std::uint16_t);
    explicit TcpClient(std::string&&, const std::uint16_t);

    std::expected<void,          TcpError> connect_to_server();
    std::expected<std::uint64_t, TcpError> send(const void*, size_t);
    std::expected<std::uint64_t, TcpError> recv(void*, size_t);
};
} // End namespace fiah::io