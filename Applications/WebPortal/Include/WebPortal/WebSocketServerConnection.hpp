#ifndef NEXUS_WEB_SOCKET_SERVER_CONNECTION_HPP
#define NEXUS_WEB_SOCKET_SERVER_CONNECTION_HPP
#include <memory>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/WebServices/WebSocketChannel.hpp>

namespace Nexus {

  /**
   * Adapts WebSocket channels received from HTTP upgrades into a
   * ServerConnection that can be used by a ServiceProtocolServer.
   */
  class WebSocketServerConnection {
    public:
      using Channel =
        Beam::WebSocketChannel<std::shared_ptr<Beam::TcpSocketChannel>>;

      /**
       * Feeds a WebSocket channel into the accept queue.
       * @param channel The upgraded WebSocket channel.
       */
      void push(std::unique_ptr<Channel> channel);

      std::unique_ptr<Channel> accept();
      void close();

    private:
      Beam::Queue<std::shared_ptr<std::unique_ptr<Channel>>> m_channels;
  };

  inline void WebSocketServerConnection::push(
      std::unique_ptr<Channel> channel) {
    m_channels.push(
      std::make_shared<std::unique_ptr<Channel>>(std::move(channel)));
  }

  inline std::unique_ptr<WebSocketServerConnection::Channel>
      WebSocketServerConnection::accept() {
    return std::move(*m_channels.pop());
  }

  inline void WebSocketServerConnection::close() {
    m_channels.close(Beam::EndOfFileException());
  }
}

#endif
