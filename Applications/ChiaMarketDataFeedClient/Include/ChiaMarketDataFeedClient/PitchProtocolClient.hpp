#ifndef NEXUS_PITCH_PROTOCOL_CLIENT_HPP
#define NEXUS_PITCH_PROTOCOL_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "ChiaMarketDataFeedClient/PitchMessage.hpp"

namespace Nexus {

  /**
   * Parses PITCH messages from the PITCH feed.
   * @param <C> The type of Channel receiving the PITCH messages.
   */
  template<typename C>
  class PitchProtocolClient {
    public:

      /** The type of channel receiving the market data feed. */
      using Channel = Beam::dereference_t<C>;

      /**
       * Constructs a PitchProtocolClient.
       * @param channel The Channel receiving the market data feed.
       */
      template<typename CF>
      explicit PitchProtocolClient(CF&& channel);

      ~PitchProtocolClient();

      /** Reads the next message from the feed. */
      PitchMessage read();

      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::SharedBuffer m_buffer;
      const char* m_cursor;
      std::uint16_t m_remaining_size;
      std::uint32_t m_sequence_number;
      Beam::OpenState m_open_state;
  };

  template<typename C>
  template<typename CF>
  PitchProtocolClient<C>::PitchProtocolClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_cursor(nullptr),
          m_remaining_size(0),
          m_sequence_number(0) {}
    catch (const std::exception&) {
      std::throw_with_nested(Beam::ConnectException(
        "Failed to initialize the PITCH protocol client."));
    }

  template<typename C>
  PitchProtocolClient<C>::~PitchProtocolClient() {
    close();
  }

  template<typename C>
  PitchMessage PitchProtocolClient<C>::read() {
    return Beam::try_or_nest([&] {
      while(m_remaining_size == 0) {
        static const auto LENGTH_SIZE = 2;
        static const auto COUNT_SIZE = 1;
        static const auto UNIT_SIZE = 1;
        static const auto SEQUENCE_SIZE = 4;
        static const auto HEADER_SIZE =
          LENGTH_SIZE + COUNT_SIZE + UNIT_SIZE + SEQUENCE_SIZE;
        reset(m_buffer);
        m_channel->get_reader().read(Beam::out(m_buffer));
        m_cursor = m_buffer.get_data();
        m_remaining_size = boost::endian::little_to_native(
          *reinterpret_cast<const std::uint16_t*>(m_cursor)) - HEADER_SIZE;
        m_cursor += LENGTH_SIZE;
        m_cursor += COUNT_SIZE;
        auto unit = boost::endian::little_to_native(
          *reinterpret_cast<const std::uint8_t*>(m_cursor));
        if(unit == 0) {
          m_remaining_size = 0;
          continue;
        }
        m_cursor += UNIT_SIZE;
        auto sequence_number = boost::endian::little_to_native(
          *reinterpret_cast<const std::uint32_t*>(m_cursor));
        if(sequence_number != 0) {
          m_sequence_number = sequence_number;
        }
        m_cursor += SEQUENCE_SIZE;
      }
      auto message =
        PitchMessage::parse(Beam::out(m_cursor), m_remaining_size);
      m_remaining_size -= message.m_length;
      m_cursor += message.m_length - 2;
      return message;
    }, Beam::IOException("Failed to read PITCH message."));
  }

  template<typename C>
  void PitchProtocolClient<C>::close() {
    if (m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_open_state.close();
  }
}

#endif
