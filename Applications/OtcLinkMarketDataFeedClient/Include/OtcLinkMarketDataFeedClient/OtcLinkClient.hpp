#ifndef OTC_LINK_CLIENT_HPP
#define OTC_LINK_CLIENT_HPP
#include <cstdint>
#include <iostream>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkMessage.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkPacket.hpp"

namespace Nexus {

  /**
   * Implements a client using the OTC Link message protocol.
   * @tparam C The type of Channel connected to the OTC Link server.
   */
  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  class OtcLinkClient {
    public:

      /** The type of Channel connected to the OTC Link server. */
      using Channel = Beam::dereference_t<C>;

      /**
       * Constructs an OtcLinkClient.
       * @param channel The Channel to connect to the OTC Link server.
       */
      template<Beam::Initializes<C> CF>
      explicit OtcLinkClient(CF&& channel);

      ~OtcLinkClient();

      /** Reads the next message from the feed. */
      OtcLinkMessage read();

      /**
       * Reads the next message from the feed.
       * @param sequence_number The message's sequence number.
       */
      OtcLinkMessage read(Beam::Out<std::uint32_t> sequence_number);

      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::SharedBuffer m_buffer;
      OtcLinkPacket m_packet;
      const char* m_source;
      std::size_t m_remaining_size;
      std::uint8_t m_remaining_messages;
      std::uint32_t m_sequence_number;
      Beam::OpenState m_open_state;

      OtcLinkClient(const OtcLinkClient&) = delete;
      OtcLinkClient& operator =(const OtcLinkClient&) = delete;
  };

  template<typename C>
  OtcLinkClient(C&&) -> OtcLinkClient<std::remove_cvref_t<C>>;

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  OtcLinkClient<C>::OtcLinkClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_remaining_messages(0),
          m_sequence_number(-1) {
    } catch(const std::exception&) {
      std::throw_with_nested(
        Beam::ConnectException("OTC Link client failed to connect."));
    }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  OtcLinkClient<C>::~OtcLinkClient() {
    close();
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  OtcLinkMessage OtcLinkClient<C>::read() {
    auto sequence_number = std::uint32_t();
    return read(Beam::out(sequence_number));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  OtcLinkMessage OtcLinkClient<C>::read(
      Beam::Out<std::uint32_t> sequence_number) {
    if(m_remaining_messages == 0) {
      while(true) {
        reset(m_buffer);
        Beam::try_or_nest([&] {
          m_channel->get_reader().read(Beam::out(m_buffer));
          m_packet = OtcLinkPacket::parse(
            std::string_view(m_buffer.get_data(), m_buffer.get_size()));
        }, Beam::IOException("Failed to read OTC Link packet."));
        if(m_packet.m_message_count != 0) {
          if(m_sequence_number != static_cast<std::uint32_t>(-1) &&
              m_packet.m_sequence_number != m_sequence_number) {
            std::cout << boost::posix_time::microsec_clock::universal_time() <<
              ": packets dropped (" << m_sequence_number << " - " <<
                (m_packet.m_sequence_number - 1) << ')' << std::endl;
          }
          m_sequence_number = m_packet.m_sequence_number + 1;
          m_source = m_packet.m_payload;
          m_remaining_size = m_buffer.get_size() - OtcLinkPacket::HEADER_LENGTH;
          m_remaining_messages = m_packet.m_message_count;
          break;
        }
      }
    }
    auto message = Beam::try_or_nest([&] {
      return OtcLinkMessage::parse(
        std::string_view(m_source, m_remaining_size));
    }, Beam::IOException("Failed to read OTC Link message."));
    m_remaining_size -= message.m_size;
    m_source += message.m_size;
    --m_remaining_messages;
    *sequence_number = m_packet.m_sequence_number;
    return message;
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void OtcLinkClient<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_open_state.close();
  }
}

#endif
