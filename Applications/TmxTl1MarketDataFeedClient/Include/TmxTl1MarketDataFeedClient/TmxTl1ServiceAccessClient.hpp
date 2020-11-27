#ifndef NEXUS_TMX_TL1_SERVICE_ACCESS_CLIENT_HPP
#define NEXUS_TMX_TL1_SERVICE_ACCESS_CLIENT_HPP
#include <cstdint>
#include <deque>
#include <functional>
#include <vector>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/StampProtocol/StampPacket.hpp"

namespace Nexus::MarketDataService {

  /**
   * Produces fixed-width messages received from a TMX TL1 service.
   * @param <C> The type of Channel receiving the market data feed.
   */
  template<typename C>
  class TmxTl1ServiceAccessClient {
    public:

      /** The type of channel receiving the market data feed. */
      using Channel = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a TmxTl1ServiceAccessClient.
       * @param channel The Channel receiving the market data feed.
       */
      template<typename CF>
      explicit TmxTl1ServiceAccessClient(CF&& channel);

      ~TmxTl1ServiceAccessClient();

      /** Reads the next message from the feed. */
      StampProtocol::StampPacket Read();

      void Close();

    private:
      struct BufferEntry {
        Beam::IO::SharedBuffer m_buffer;
        std::uint32_t m_sequenceNumber;

        BufferEntry(Beam::IO::SharedBuffer buffer,
          std::uint32_t sequenceNumber);
      };
      Beam::GetOptionalLocalPtr<C> m_channel;
      std::uint32_t m_sequenceNumber;
      std::vector<Beam::IO::SharedBuffer> m_buffers;
      std::deque<BufferEntry> m_pendingBuffers;
      Beam::IO::OpenState m_openState;

      TmxTl1ServiceAccessClient(const TmxTl1ServiceAccessClient&) = delete;
      TmxTl1ServiceAccessClient& operator =(
        const TmxTl1ServiceAccessClient&) = delete;
      void AddPendingBuffer(Beam::IO::SharedBuffer buffer,
        std::size_t sequenceNumber);
  };

  template<typename C>
  TmxTl1ServiceAccessClient<C>::BufferEntry::BufferEntry(
    Beam::IO::SharedBuffer buffer, std::uint32_t sequenceNumber)
    : m_buffer(std::move(buffer)),
      m_sequenceNumber(sequenceNumber) {}

  template<typename C>
  template<typename CF>
  TmxTl1ServiceAccessClient<C>::TmxTl1ServiceAccessClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_sequenceNumber(0) {
    } catch(const std::exception&) {
      std::throw_with_nested(Beam::IO::ConnectException(
        "Failed to initialize the TMX TL1 service access client."));
    }

  template<typename C>
  TmxTl1ServiceAccessClient<C>::~TmxTl1ServiceAccessClient() {
    Close();
  }

  template<typename C>
  StampProtocol::StampPacket TmxTl1ServiceAccessClient<C>::Read() {
    static const auto HEARTBEAT_MESSAGE_TYPE = Beam::FixedString<2>("V ");
    return Beam::TryOrNest([&] {
      auto bufferIndex = std::size_t(0);
      while(true) {
        if(m_buffers.size() <= bufferIndex) {
          m_buffers.emplace_back();
        }
        auto& buffer = m_buffers[bufferIndex];
        buffer.Reset();
        if(m_pendingBuffers.empty()) {
          m_channel->GetReader().Read(Beam::Store(buffer));
        } else {
          buffer = std::move(m_pendingBuffers.front().m_buffer);
          m_pendingBuffers.pop_front();
        }
        auto packet = StampProtocol::StampPacket::Parse(buffer.GetData(),
          buffer.GetSize());
        if(packet.m_header.m_messageType == HEARTBEAT_MESSAGE_TYPE) {
          continue;
        }
        if(m_sequenceNumber == 0) {
          if(packet.m_header.m_continuationIndicator ==
              StampProtocol::ContinuationIndicator::STAND_ALONE ||
              packet.m_header.m_continuationIndicator ==
              StampProtocol::ContinuationIndicator::SPANNING) {
            m_sequenceNumber = packet.m_header.m_sequenceNumber;
          } else {
            continue;
          }
        } else if(packet.m_header.m_sequenceNumber == m_sequenceNumber + 1) {
          ++m_sequenceNumber;
        } else if(packet.m_header.m_sequenceNumber <= m_sequenceNumber) {
          continue;
        } else {
          std::cout << "Dropped packets: " << m_sequenceNumber + 1 << " - " <<
            packet.m_header.m_sequenceNumber - 1 << std::endl;
          AddPendingBuffer(buffer, packet.m_header.m_sequenceNumber);
          m_sequenceNumber = 0;
          bufferIndex = 0;
          continue;
        }
        if(packet.m_header.m_continuationIndicator ==
            StampProtocol::ContinuationIndicator::STAND_ALONE) {
          return packet;
        } else if(packet.m_header.m_continuationIndicator ==
            StampProtocol::ContinuationIndicator::SPANNING) {
          std::cout << "Spanning" << std::endl;
          bufferIndex = 1;
        } else if(packet.m_header.m_continuationIndicator ==
            StampProtocol::ContinuationIndicator::SPANNING_CONTINUATION) {
          std::cout << "Spanning Continuation" << std::endl;
          ++bufferIndex;
        } else if(packet.m_header.m_continuationIndicator ==
            StampProtocol::ContinuationIndicator::CONTINUATION) {
          std::cout << "Continuation" << std::endl;
          bufferIndex = 0;
        }
      }
    }, Beam::IO::IOException("Failed to read STAMP message."));
  }

  template<typename C>
  void TmxTl1ServiceAccessClient<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_buffers.clear();
    m_openState.Close();
  }

  template<typename C>
  void TmxTl1ServiceAccessClient<C>::AddPendingBuffer(
      Beam::IO::SharedBuffer buffer, std::size_t sequenceNumber) {
    auto entry = BufferEntry(std::move(buffer), sequenceNumber);
    auto pendingBufferIterator = std::lower_bound(m_pendingBuffers.begin(),
      m_pendingBuffers.end(), entry,
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_sequenceNumber < rhs.m_sequenceNumber;
      });
    if(pendingBufferIterator == m_pendingBuffers.end() ||
        pendingBufferIterator->m_sequenceNumber != sequenceNumber) {
      m_pendingBuffers.insert(pendingBufferIterator, entry);
    }
  }
}

#endif
