#pragma once

#include <cstdint>
#include <expected>

#include "fiah/io/Tcp.hh"
#include "fiah/io/TcpError.hh"
#include "fiah/utils/Logger.hh"
#include "fiah/utils/Timer.hpp"

namespace fiah::io
{

/// @brief TCP server implementation using POSIX internet sockets.
///        There is no Windows support; currently or ever.
/// @attention Guaranteed to not throw
class TcpServer : public Tcp
{
private:
    static constexpr std::uint16_t MAX_LISTEN_NUM{ 10 };
    static inline utils::Logger<TcpServer> &m_logger{
        utils::Logger<TcpServer>::get_instance ("TcpServer")
    };

public:
    TcpServer () = default;
    explicit TcpServer (const std::string &, std::uint16_t) noexcept;
    explicit TcpServer (std::string &&, std::uint16_t);

    std::expected<void, TcpError> start ();
    std::expected<SocketRAII, TcpError> accept_client ();

    __always_inline [[gnu::hot]] auto
    send (SocketRAII &client, const void *buf, size_t len)
      -> std::expected<std::uint64_t, TcpError>
    {
        utils::Timer timer{ "TcpServer::send()" };
        ssize_t result = ::send (client, buf, len, MSG_NOSIGNAL);
        if (result < 0) [[unlikely]]
            return std::unexpected (TcpError::SEND_FAIL);
        return static_cast<std::uint64_t> (result);
    }

    __always_inline [[gnu::hot]] auto
    recv (SocketRAII &client, void *buf, size_t len)
      -> std::expected<std::uint64_t, TcpError>
    {
        utils::Timer timer{ "TcpServer::recv()" };
        ssize_t result = ::recv (client, buf, len, 0);
        if (result < 0) [[unlikely]]
            return std::unexpected (TcpError::RECV_FAIL);
        return static_cast<std::uint64_t> (result);
    }
};
} // End namespace fiah::io