#ifndef NEXUS_TMXTL1SERVICEACCESSCLIENT_HPP
#define NEXUS_TMXTL1SERVICEACCESSCLIENT_HPP
#include <cstdint>
#include <deque>
#include <functional>
#include <vector>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/StampProtocol/StampPacket.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class TmxTl1ServiceAccessClient
      \brief Produces fixed-width messages received from a TMX TL1 service.
      \tparam FeedChannelType The type of Channel receiving the market data
              feed.
   */
  template<typename FeedChannelType>
  class TmxTl1ServiceAccessClient : private boost::noncopyable {
    public:

      //! The type of channel receiving the market data feed.
      typedef typename Beam::TryDereferenceType<FeedChannelType>::type
        FeedChannel;

      //! Constructs a TmxTl1ServiceAccessClient.
      /*!
        \param feedChannel The Channel receiving the market data feed.
      */
      template<typename FeedChannelForward>
      TmxTl1ServiceAccessClient(FeedChannelForward&& feedChannel);

      ~TmxTl1ServiceAccessClient();

      //! Reads the next message from the feed.
      StampProtocol::StampPacket Read();

      void Open();

      void Close();

    private:
      typedef typename FeedChannel::Reader::Buffer FeedBuffer;
      struct BufferEntry {
        FeedBuffer m_buffer;
        std::uint32_t m_sequenceNumber;

        BufferEntry(FeedBuffer buffer, std::uint32_t sequenceNumber);
      };
      typename Beam::OptionalLocalPtr<FeedChannelType>::type m_feedChannel;
      std::uint32_t m_sequenceNumber;
      std::vector<FeedBuffer> m_buffers;
      std::deque<BufferEntry> m_pendingBuffers;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void AddPendingBuffer(FeedBuffer buffer, std::size_t sequenceNumber);
  };

  template<typename FeedChannelType>
  TmxTl1ServiceAccessClient<FeedChannelType>::BufferEntry::BufferEntry(
      FeedBuffer buffer, std::uint32_t sequenceNumber)
      : m_buffer(std::move(buffer)),
        m_sequenceNumber(sequenceNumber) {}

  template<typename FeedChannelType>
  template<typename FeedChannelForward>
  TmxTl1ServiceAccessClient<FeedChannelType>::TmxTl1ServiceAccessClient(
      FeedChannelForward&& feedChannel)
      : m_feedChannel(std::forward<FeedChannelType>(feedChannel)) {}

  template<typename FeedChannelType>
  TmxTl1ServiceAccessClient<FeedChannelType>::~TmxTl1ServiceAccessClient() {
    Close();
  }

  template<typename FeedChannelType>
  StampProtocol::StampPacket
      TmxTl1ServiceAccessClient<FeedChannelType>::Read() {
    static const Beam::FixedString<2> HEARTBEAT_MESSAGE_TYPE = "V ";
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
    }
    std::size_t bufferIndex = 0;
    while(true) {
      if(m_buffers.size() <= bufferIndex) {
        m_buffers.emplace_back();
      }
      FeedBuffer& buffer = m_buffers[bufferIndex];
      buffer.Reset();
      if(m_pendingBuffers.empty()) {
        m_feedChannel->GetReader().Read(Beam::Store(buffer));
      } else {
        buffer = std::move(m_pendingBuffers.front().m_buffer);
        m_pendingBuffers.pop_front();
      }
      StampProtocol::StampPacket packet = StampProtocol::StampPacket::Parse(
        buffer.GetData(), buffer.GetSize());
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
  }

  template<typename FeedChannelType>
  void TmxTl1ServiceAccessClient<FeedChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_feedChannel->GetConnection().Open();
      m_sequenceNumber = 0;
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename FeedChannelType>
  void TmxTl1ServiceAccessClient<FeedChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename FeedChannelType>
  void TmxTl1ServiceAccessClient<FeedChannelType>::Shutdown() {
    m_feedChannel->GetConnection().Close();
    m_openState.SetClosed();
    m_buffers.clear();
  }

  template<typename FeedChannelType>
  void TmxTl1ServiceAccessClient<FeedChannelType>::AddPendingBuffer(
      FeedBuffer buffer, std::size_t sequenceNumber) {
    BufferEntry entry(std::move(buffer), sequenceNumber);
    auto pendingBufferIterator = std::lower_bound(m_pendingBuffers.begin(),
      m_pendingBuffers.end(), entry,
      [] (const BufferEntry& lhs, const BufferEntry& rhs) {
        return lhs.m_sequenceNumber < rhs.m_sequenceNumber;
      });
    if(pendingBufferIterator == m_pendingBuffers.end() ||
        pendingBufferIterator->m_sequenceNumber != sequenceNumber) {
      m_pendingBuffers.insert(pendingBufferIterator, entry);
    }
  }
}
}

#endif
