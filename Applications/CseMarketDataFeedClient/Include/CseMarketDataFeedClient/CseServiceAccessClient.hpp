#ifndef NEXUS_CSE_SERVICE_ACCESS_CLIENT_HPP
#define NEXUS_CSE_SERVICE_ACCESS_CLIENT_HPP
#include <cstdint>
#include <deque>
#include <functional>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/StampProtocol/StampMessage.hpp"
#include "Nexus/StampProtocol/StampPacket.hpp"
#include "CseMarketDataFeedClient/CseMarketDataFeedClient.hpp"

namespace Nexus::MarketDataService {

  /** Stores the configuration used for a CseServiceAccessClient. */
  struct CseServiceAccessConfiguration {

    /** Whether retransmission is enabled. */
    bool m_enableRetransmission;

    /** The maximum number of retransmissions to perform. */
    int m_maxRetransmissionCount;

    /** The size of the largest possible retransmission block. */
    std::size_t m_maxRetransmissionBlock;

    /** Constructs a CseServiceAccessConfiguration with default values. */
    CseServiceAccessConfiguration();
  };

  /**
   * Produces StampMessages received from a CSE data feed.
   * @param <F> The type of Channel receiving the market data feed.
   * @param <C> The type of Channel used to send retransmission requests.
   * @param <S> The type of Channel used to receive retransmission messages.
   */
  template<typename F, typename C, typename S>
  class CseServiceAccessClient : private boost::noncopyable {
    public:

      /** The type of channel receiving the market data feed. */
      using FeedChannel = Beam::GetTryDereferenceType<F>;

      /** The type of Channel used to send retransmission requests. */
      using RetransmissionClientChannel = Beam::GetTryDereferenceType<C>;

      /** The type of Channel used to receive retransmission messages. */
      using RetransmissionServerChannel = Beam::GetTryDereferenceType<S>;

      /**
       * The type of function used to build instances of the
       * RetransmissionClientChannel.
       * @param channel Stores the Channel to build.
       */
      using RetransmissionClientChannelBuilder = std::function<void (
        Beam::Out<std::optional<RetransmissionClientChannel>> channel)>;

      /**
       * Constructs a CseServiceAccessClient.
       * @param config The configuration to use.
       * @param feedChannel The Channel receiving the market data feed.
       * @param retransmissionClientChannelBuilder Builds instances of the
       *        Channel used to send retransmission requests.
       * @param retransmissionServerChannel The Channel receiving retransmission
       *        messages.
       */
      template<typename FF, typename SF>
      CseServiceAccessClient(CseServiceAccessConfiguration config,
        FF&& feedChannel,
        RetransmissionClientChannelBuilder retransmissionClientChannelBuilder,
        SF&& retransmissionServerChannel);

      ~CseServiceAccessClient();

      /** Reads the next message from the feed. */
      StampProtocol::StampMessage Read();

      void Close();

    private:
      using FeedBuffer = typename FeedChannel::Reader::Buffer;
      struct BufferEntry {
        FeedBuffer m_buffer;
        std::uint32_t m_sequenceNumber;

        BufferEntry(FeedBuffer buffer, std::uint32_t sequenceNumber);
      };
      CseServiceAccessConfiguration m_config;
      Beam::GetOptionalLocalPtr<F> m_feedChannel;
      RetransmissionClientChannelBuilder m_retransmissionClientChannelBuilder;
      Beam::GetOptionalLocalPtr<S> m_retransmissionServerChannel;
      int m_retransmissionCount;
      std::uint32_t m_sequenceNumber;
      std::vector<FeedBuffer> m_buffers;
      std::deque<BufferEntry> m_pendingBuffers;
      Beam::IO::OpenState m_openState;

      template<typename Buffer>
      static void BuildRetransmissionRequestBuffer(Beam::Out<Buffer> buffer,
        std::size_t startSequenceNumber, std::size_t endSequenceNumber);
      void AddPendingBuffer(FeedBuffer buffer, std::size_t sequenceNumber);
      void SendRetransmissionRequest(std::size_t startSequenceNumber,
        std::size_t endSequenceNumber);
      void ReadRetransmissionResponse(std::size_t startSequenceNumber,
        std::size_t endSequenceNumber);
      void Retransmit(std::size_t startSequenceNumber,
        std::size_t endSequenceNumber);
  };

  inline CseServiceAccessConfiguration::CseServiceAccessConfiguration()
    : m_enableRetransmission(false),
      m_maxRetransmissionCount(100),
      m_maxRetransmissionBlock(20000) {}

  template<typename F, typename C, typename S>
  CseServiceAccessClient<F, C, S>::BufferEntry::BufferEntry(FeedBuffer buffer,
    std::uint32_t sequenceNumber)
    : m_buffer(std::move(buffer)),
      m_sequenceNumber(sequenceNumber) {}

  template<typename F, typename C, typename S>
  template<typename FF, typename SF>
  CseServiceAccessClient<F, C, S>::CseServiceAccessClient(
    CseServiceAccessConfiguration config, FF&& feedChannel,
    RetransmissionClientChannelBuilder retransmissionClientChannelBuilder,
    SF&& retransmissionServerChannel)
    : m_config(std::move(config)),
      m_feedChannel(std::forward<F>(feedChannel)),
      m_retransmissionClientChannelBuilder(
        std::move(retransmissionClientChannelBuilder)),
      m_retransmissionServerChannel(std::forward<SF>(
        retransmissionServerChannel)),
      m_retransmissionCount(0),
      m_sequenceNumber(0) {}

  template<typename F, typename C, typename S>
  CseServiceAccessClient<F, C, S>::~CseServiceAccessClient() {
    Close();
  }

  template<typename F, typename C, typename S>
  StampProtocol::StampMessage CseServiceAccessClient<F, C, S>::Read() {
    static const auto HEARTBEAT_MESSAGE_TYPE = Beam::FixedString<2>("V ");
    m_openState.EnsureOpen();
    auto packet = StampProtocol::StampPacket();
    if(m_config.m_enableRetransmission) {
      auto retransmissionBuffer =
        typename RetransmissionServerChannel::Reader::Buffer();
      while(m_retransmissionServerChannel->GetReader().IsDataAvailable()) {
        retransmissionBuffer.Reset();
        m_retransmissionServerChannel->GetReader().Read(
          Beam::Store(retransmissionBuffer));
      }
    }
    auto bufferIndex = std::size_t(0);
    while(true) {
      if(m_buffers.size() <= bufferIndex) {
        m_buffers.emplace_back();
      }
      auto& buffer = m_buffers[bufferIndex];
      buffer.Reset();
      if(m_pendingBuffers.empty()) {
        m_feedChannel->GetReader().Read(Beam::Store(buffer));
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
        if(m_config.m_enableRetransmission) {
          try {
            Retransmit(m_sequenceNumber + 1,
              packet.m_header.m_sequenceNumber - 1);
          } catch(const std::exception&) {
            m_sequenceNumber = 0;
            bufferIndex = 0;
          }
        } else {
          m_sequenceNumber = 0;
          bufferIndex = 0;
        }
        continue;
      }
      if(packet.m_header.m_continuationIndicator ==
          StampProtocol::ContinuationIndicator::STAND_ALONE) {
        auto message = StampProtocol::StampMessage(packet.m_header,
          packet.m_message, packet.m_messageSize);
        return message;
      } else if(packet.m_header.m_continuationIndicator ==
          StampProtocol::ContinuationIndicator::SPANNING) {
        bufferIndex = 1;
      } else if(packet.m_header.m_continuationIndicator ==
          StampProtocol::ContinuationIndicator::SPANNING_CONTINUATION) {
        ++bufferIndex;
      } else if(packet.m_header.m_continuationIndicator ==
          StampProtocol::ContinuationIndicator::CONTINUATION) {
        bufferIndex = 0;
      }
    }
  }

  template<typename F, typename C, typename S>
  void CseServiceAccessClient<F, C, S>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    if(m_config.m_enableRetransmission) {
      m_retransmissionServerChannel->GetConnection().Close();
    }
    m_feedChannel->GetConnection().Close();
    m_buffers.clear();
    m_openState.Close();
  }

  template<typename F, typename C, typename S>
  template<typename Buffer>
  void CseServiceAccessClient<F, C, S>::BuildRetransmissionRequestBuffer(
      Beam::Out<Buffer> buffer, std::size_t startSequenceNumber,
      std::size_t endSequenceNumber) {
    static constexpr auto SEQUENCE_NUMBER_SIZE = std::size_t(9);
    buffer->Append("SEQN", 4);
    auto messageStartNumber = boost::lexical_cast<std::string>(
      startSequenceNumber);
    while(messageStartNumber.size() < SEQUENCE_NUMBER_SIZE) {
      messageStartNumber.insert(messageStartNumber.begin(), '0');
    }
    buffer->Append(messageStartNumber.c_str(), SEQUENCE_NUMBER_SIZE);
    auto messageEndNumber = boost::lexical_cast<std::string>(endSequenceNumber);
    while(messageEndNumber.size() < SEQUENCE_NUMBER_SIZE) {
      messageEndNumber.insert(messageEndNumber.begin(), '0');
    }
    buffer->Append(messageEndNumber.c_str(), SEQUENCE_NUMBER_SIZE);
    buffer->Append('\n');
  }

  template<typename F, typename C, typename S>
  void CseServiceAccessClient<F, C, S>::AddPendingBuffer(FeedBuffer buffer,
      std::size_t sequenceNumber) {
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

  template<typename F, typename C, typename S>
  void CseServiceAccessClient<F, C, S>::SendRetransmissionRequest(
      std::size_t startSequenceNumber, std::size_t endSequenceNumber) {
    static constexpr auto RETRANSMISSION_RESPONSE_SIZE = std::size_t(151);
    auto retransmissionRequestBuffer =
      typename RetransmissionClientChannel::Writer::Buffer();
    BuildRetransmissionRequestBuffer(Beam::Store(retransmissionRequestBuffer),
      startSequenceNumber, endSequenceNumber);
    auto retransmissionClientChannel =
      std::optional<RetransmissionClientChannel>();
    m_retransmissionClientChannelBuilder(
      Beam::Store(retransmissionClientChannel));
    retransmissionClientChannel->GetWriter().Write(retransmissionRequestBuffer);
    auto retransmissionResponseBuffer =
      typename RetransmissionClientChannel::Reader::Buffer();
    while(retransmissionResponseBuffer.GetSize() <
        RETRANSMISSION_RESPONSE_SIZE) {
      retransmissionClientChannel->GetReader().Read(
        Beam::Store(retransmissionResponseBuffer));
    }
    retransmissionResponseBuffer.Reset();
    try {
      retransmissionClientChannel->GetReader().Read(
        Beam::Store(retransmissionResponseBuffer));
    } catch(const std::exception&) {}
  }

  template<typename F, typename C, typename S>
  void CseServiceAccessClient<F, C, S>::ReadRetransmissionResponse(
      std::size_t startSequenceNumber, std::size_t endSequenceNumber) {
    auto packet = StampProtocol::StampPacket();
    auto retransmissionBuffer =
      typename RetransmissionServerChannel::Reader::Buffer();
    while(m_retransmissionServerChannel->GetReader().IsDataAvailable()) {
      retransmissionBuffer.Reset();
      m_retransmissionServerChannel->GetReader().Read(
        Beam::Store(retransmissionBuffer));
      auto packet = StampProtocol::StampPacket::Parse(
        retransmissionBuffer.GetData(), retransmissionBuffer.GetSize());
      if(packet.m_header.m_sequenceNumber < startSequenceNumber ||
          packet.m_header.m_sequenceNumber > endSequenceNumber) {
        continue;
      }
      std::cout << "Recovered: " << packet.m_header.m_sequenceNumber <<
        std::endl;
      AddPendingBuffer(retransmissionBuffer, packet.m_header.m_sequenceNumber);
    }
  }

  template<typename F, typename C, typename S>
  void CseServiceAccessClient<F, C, S>::Retransmit(
      std::size_t startSequenceNumber, std::size_t endSequenceNumber) {
    while(startSequenceNumber <= endSequenceNumber) {
      if(m_retransmissionCount > m_config.m_maxRetransmissionCount) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Too many retransmissions"));
      }
      auto endBlock = std::min(endSequenceNumber, startSequenceNumber +
        m_config.m_maxRetransmissionBlock - 1);
      ++m_retransmissionCount;
      SendRetransmissionRequest(startSequenceNumber, endBlock);
      ReadRetransmissionResponse(startSequenceNumber, endBlock);
      startSequenceNumber = endBlock + 1;
    }
  }
}

#endif
