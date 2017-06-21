#ifndef NEXUS_CHIAMDPROTOCOLCLIENT_HPP
#define NEXUS_CHIAMDPROTOCOLCLIENT_HPP
#include <string>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "ChiaMarketDataFeedClient/ChiaMessage.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class ChiaMdProtocolClient
      \brief Parses packets from the CHIA market data feed.
      \tparam ChannelType The type of Channel receiving data.
   */
  template<typename ChannelType>
  class ChiaMdProtocolClient : private boost::noncopyable {
    public:

      //! The type of Channel receiving data.
      using Channel = Beam::GetTryDereferenceType<ChannelType>;

      //! Constructs a ChiaMdProtocolClient.
      /*!
        \param channel Initializes the Channel receiving data.
        \param username The username.
        \param password The password.
      */
      template<typename ChannelForward>
      ChiaMdProtocolClient(ChannelForward&& channel,
        std::string username, std::string password);

      ~ChiaMdProtocolClient();

      //! Reads the next message.
      ChiaMessage Read();

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<ChannelType> m_channel;
      std::string m_username;
      std::string m_password;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  ChiaMdProtocolClient<ChannelType>::ChiaMdProtocolClient(
      ChannelForward&& channel, std::string username, std::string password)
      : m_client{std::forward<ChannelForward>(channel)},
        m_username{std::move(username)},
        m_password{std::move(password)} {}

  template<typename ChannelType>
  ChiaMdProtocolClient<ChannelType>::~ChiaMdProtocolClient() {
    Close();
  }

  template<typename ChannelType>
  ChiaMessage ChiaMdProtocolClient<ChannelType>::Read() {
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException{});
    }
    return {};
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_channel->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void ChiaMdProtocolClient<ChannelType>::Shutdown() {
    m_channel->Close();
    m_openState.SetClosed();
  }
}
}

#endif
