#ifndef NEXUS_PITCH_PROTOCOL_CLIENT_HPP
#define NEXUS_PITCH_PROTOCOL_CLIENT_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include "ChiaMarketDataFeedClient/PitchMessage.hpp"

namespace Nexus::MarketDataService {

  /**
   * Parses PITCH messages from the PITCH feed.
   * @param <C> The type of Channel receiving the PITCH messages.
   */
  template<typename C>
  class PitchProtocolClient {
    public:

      /** The type of channel receiving the market data feed. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a PitchProtocolClient.
       * @param channel The Channel receiving the market data feed.
       */
      template<typename CF>
      explicit PitchProtocolClient(CF&& channel);

      ~PitchProtocolClient();

      /** Reads the next message from the feed. */
      PitchMessage Read();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<C> m_channel;
      typename Channel::Reader::Buffer m_buffer;
      const char* m_cursor;
      std::uint16_t m_remainingSize;
      std::uint32_t m_sequenceNumber;
      Beam::IO::OpenState m_openState;
  };

  template<typename C>
  template<typename CF>
  PitchProtocolClient<C>::PitchProtocolClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_cursor(nullptr),
          m_remainingSize(0),
          m_sequenceNumber(0) {}
    catch (const std::exception&) {
      std::throw_with_nested(Beam::IO::ConnectException(
        "Failed to initialize the PITCH protocol client."));
    }

  template<typename C>
  PitchProtocolClient<C>::~PitchProtocolClient() {
    Close();
  }

  template<typename C>
  PitchMessage PitchProtocolClient<C>::Read() {
    return Beam::TryOrNest([&] {
      while(m_remainingSize == 0) {
        static const auto LENGTH_SIZE = 2;
        static const auto COUNT_SIZE = 1;
        static const auto UNIT_SIZE = 1;
        static const auto SEQUENCE_SIZE = 4;
        static const auto HEADER_SIZE =
          LENGTH_SIZE + COUNT_SIZE + UNIT_SIZE + SEQUENCE_SIZE;
        m_buffer.Reset();
        m_channel->GetReader().Read(Beam::Store(m_buffer));
        m_cursor = m_buffer.GetData();
        m_remainingSize = Beam::FromLittleEndian(
          *reinterpret_cast<const std::uint16_t*>(m_cursor)) - HEADER_SIZE;
        m_cursor += LENGTH_SIZE;
        m_cursor += COUNT_SIZE;
        auto unit = Beam::FromLittleEndian(
          *reinterpret_cast<const std::uint8_t*>(m_cursor));
        if(unit == 0) {
          m_remainingSize = 0;
          continue;
        }
        m_cursor += UNIT_SIZE;
        m_sequenceNumber = Beam::FromLittleEndian(
          *reinterpret_cast<const std::uint32_t*>(m_cursor));
        m_cursor += SEQUENCE_SIZE;
      }
      auto message =
        PitchMessage::Parse(Beam::Store(m_cursor), m_remainingSize);
      m_remainingSize -= message.m_length;
      m_cursor += message.m_length;
      return message;
    }, Beam::IO::IOException("Failed to read STAMP message."));
  }

  template<typename C>
  void PitchProtocolClient<C>::Close() {
    if (m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }
}

#endif
