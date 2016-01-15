#ifndef NEXUS_TMXIPSERVICEACCESSCLIENT_HPP
#define NEXUS_TMXIPSERVICEACCESSCLIENT_HPP
#include <cstdint>
#include <deque>
#include <functional>
#include <vector>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/StampProtocol/StampMessage.hpp"
#include "Nexus/StampProtocol/StampPacket.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpMarketDataFeedClient.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \struct TmxIpServiceAccessConfiguration
      \brief Stores the configuration used for a TmxIpServiceAccessClient.
   */
  struct TmxIpServiceAccessConfiguration {

    //! Whether retransmission is enabled.
    bool m_enableRetransmission;

    //! The maximum number of retransmissions to perform.
    int m_maxRetransmissionCount;

    //! The size of the largest possible retransmission block.
    std::size_t m_maxRetransmissionBlock;

    //! Constructs a TmxIpServiceAccessConfiguration with default values.
    TmxIpServiceAccessConfiguration();
  };

  /*! \class TmxIpServiceAccessClient
      \brief Produces StampMessages received from a TMX Information Processor
             service.
      \tparam FeedChannelType The type of Channel receiving the market data
              feed.
      \tparam RetransmissionClientChannelType The type of Channel used to send
              retransmission requests.
      \tparam RetransmissionServerChannelType The type of Channel used to
              receive retransmission messages.
   */
  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  class TmxIpServiceAccessClient : private boost::noncopyable {
    public:

      //! The type of channel receiving the market data feed.
      using FeedChannel = Beam::GetTryDereferenceType<FeedChannelType>;

      //! The type of Channel used to send retransmission requests.
      using RetransmissionClientChannel = Beam::GetTryDereferenceType<
        RetransmissionClientChannelType>;

      //! The type of Channel used to receive retransmission messages.
      using RetransmissionServerChannel = Beam::GetTryDereferenceType<
        RetransmissionServerChannelType>;

      //! The type of function used to build instances of the
      //! RetransmissionClientChannel.
      /*!
        \param channel Stores the Channel to build.
      */
      using RetransmissionClientChannelBuilder = std::function<void (
        Beam::Out<Beam::DelayPtr<RetransmissionClientChannel>> channel)>;

      //! Constructs a TmxIpServiceAccessClient.
      /*!
        \param config The configuration to use.
        \param feedChannel The Channel receiving the market data feed.
        \param retransmissionClientChannelBuilder Builds instances of the
               Channel used to send retransmission requests.
        \param retransmissionServerChannel The Channel receiving retransmission
               messages.
      */
      template<typename FeedChannelForward,
        typename RetransmissionServerChannelForward>
      TmxIpServiceAccessClient(const TmxIpServiceAccessConfiguration& config,
        FeedChannelForward&& feedChannel,
        RetransmissionClientChannelBuilder retransmissionClientChannelBuilder,
        RetransmissionServerChannelForward&& retransmissionServerChannel);

      ~TmxIpServiceAccessClient();

      //! Reads the next message from the feed.
      StampProtocol::StampMessage Read();

      void Open();

      void Close();

    private:
      using FeedBuffer = typename FeedChannel::Reader::Buffer;
      struct BufferEntry {
        FeedBuffer m_buffer;
        std::uint32_t m_sequenceNumber;

        BufferEntry(FeedBuffer buffer, std::uint32_t sequenceNumber);
      };
      TmxIpServiceAccessConfiguration m_config;
      Beam::GetOptionalLocalPtr<FeedChannelType> m_feedChannel;
      RetransmissionClientChannelBuilder m_retransmissionClientChannelBuilder;
      Beam::GetOptionalLocalPtr<RetransmissionServerChannelType>
        m_retransmissionServerChannel;
      int m_retransmissionCount;
      std::uint32_t m_sequenceNumber;
      std::vector<FeedBuffer> m_buffers;
      std::deque<BufferEntry> m_pendingBuffers;
      Beam::IO::OpenState m_openState;

      template<typename Buffer>
      static void BuildRetransmissionRequestBuffer(Beam::Out<Buffer> buffer,
        std::size_t startSequenceNumber, std::size_t endSequenceNumber);
      void Shutdown();
      void AddPendingBuffer(FeedBuffer buffer, std::size_t sequenceNumber);
      void SendRetransmissionRequest(std::size_t startSequenceNumber,
        std::size_t endSequenceNumber);
      void ReadRetransmissionResponse(std::size_t startSequenceNumber,
        std::size_t endSequenceNumber);
      void Retransmit(std::size_t startSequenceNumber,
        std::size_t endSequenceNumber);
  };

  inline TmxIpServiceAccessConfiguration::TmxIpServiceAccessConfiguration()
      : m_enableRetransmission(false),
        m_maxRetransmissionCount(100),
        m_maxRetransmissionBlock(20000) {}

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  TmxIpServiceAccessClient<FeedChannelType, RetransmissionClientChannelType,
      RetransmissionServerChannelType>::BufferEntry::BufferEntry(
      FeedBuffer buffer, std::uint32_t sequenceNumber)
      : m_buffer(std::move(buffer)),
        m_sequenceNumber(sequenceNumber) {}

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  template<typename FeedChannelForward,
    typename RetransmissionServerChannelForward>
  TmxIpServiceAccessClient<FeedChannelType, RetransmissionClientChannelType,
      RetransmissionServerChannelType>::TmxIpServiceAccessClient(
      const TmxIpServiceAccessConfiguration& config,
      FeedChannelForward&& feedChannel,
      RetransmissionClientChannelBuilder retransmissionClientChannelBuilder,
      RetransmissionServerChannelForward&& retransmissionServerChannel)
      : m_config(config),
        m_feedChannel(std::forward<FeedChannelType>(feedChannel)),
        m_retransmissionClientChannelBuilder(
          std::move(retransmissionClientChannelBuilder)),
        m_retransmissionServerChannel(
          std::forward<RetransmissionServerChannelForward>(
          retransmissionServerChannel)) {}

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  TmxIpServiceAccessClient<FeedChannelType, RetransmissionClientChannelType,
      RetransmissionServerChannelType>::~TmxIpServiceAccessClient() {
    Close();
  }

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  StampProtocol::StampMessage TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      Read() {
    static const Beam::FixedString<2> HEARTBEAT_MESSAGE_TYPE = "V ";
    if(!m_openState.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
    }
    StampProtocol::StampPacket packet;
    if(m_config.m_enableRetransmission) {
      typename RetransmissionServerChannel::Reader::Buffer retransmissionBuffer;
      while(m_retransmissionServerChannel->GetReader().IsDataAvailable()) {
        retransmissionBuffer.Reset();
        m_retransmissionServerChannel->GetReader().Read(
          Beam::Store(retransmissionBuffer));
      }
    }
    std::size_t bufferIndex = 0;
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
        StampProtocol::StampMessage message(packet.m_header, packet.m_message,
          packet.m_messageSize);
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

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_feedChannel->GetConnection().Open();
      if(m_config.m_enableRetransmission) {
        m_retransmissionServerChannel->GetConnection().Open();
      }
      m_retransmissionCount = 0;
      m_sequenceNumber = 0;
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  template<typename Buffer>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      BuildRetransmissionRequestBuffer(Beam::Out<Buffer> buffer,
      std::size_t startSequenceNumber, std::size_t endSequenceNumber) {
    static const std::size_t SEQUENCE_NUMBER_SIZE = 9;
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

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      Shutdown() {
    if(m_config.m_enableRetransmission) {
      m_retransmissionServerChannel->GetConnection().Close();
    }
    m_feedChannel->GetConnection().Close();
    m_openState.SetClosed();
    m_buffers.clear();
  }

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      AddPendingBuffer(FeedBuffer buffer, std::size_t sequenceNumber) {
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

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      SendRetransmissionRequest(std::size_t startSequenceNumber,
      std::size_t endSequenceNumber) {
    static const std::size_t RETRANSMISSION_RESPONSE_SIZE = 151;
    typename RetransmissionClientChannel::Writer::Buffer
      retransmissionRequestBuffer;
    BuildRetransmissionRequestBuffer(Beam::Store(retransmissionRequestBuffer),
      startSequenceNumber, endSequenceNumber);
    Beam::DelayPtr<RetransmissionClientChannel> retransmissionClientChannel;
    m_retransmissionClientChannelBuilder(
      Beam::Store(retransmissionClientChannel));
    retransmissionClientChannel->GetConnection().Open();
    retransmissionClientChannel->GetWriter().Write(retransmissionRequestBuffer);
    typename RetransmissionClientChannel::Reader::Buffer
      retransmissionResponseBuffer;
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

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      ReadRetransmissionResponse(std::size_t startSequenceNumber,
      std::size_t endSequenceNumber) {
    StampProtocol::StampPacket packet;
    typename RetransmissionServerChannel::Reader::Buffer retransmissionBuffer;
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
      AddPendingBuffer(retransmissionBuffer, packet.m_header.m_sequenceNumber);
    }
  }

  template<typename FeedChannelType, typename RetransmissionClientChannelType,
    typename RetransmissionServerChannelType>
  void TmxIpServiceAccessClient<FeedChannelType,
      RetransmissionClientChannelType, RetransmissionServerChannelType>::
      Retransmit(std::size_t startSequenceNumber,
      std::size_t endSequenceNumber) {
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
}

#endif
