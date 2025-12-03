#ifndef NEXUS_TMX_IP_SERVICE_ACCESS_CLIENT_HPP
#define NEXUS_TMX_IP_SERVICE_ACCESS_CLIENT_HPP
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
#include "Nexus/Stamp/StampMessage.hpp"
#include "Nexus/Stamp/StampPacket.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpMarketDataFeedClient.hpp"

namespace Nexus {

  /** Stores the configuration used for a TmxIpServiceAccessClient. */
  struct TmxIpServiceAccessConfiguration {

    /** Whether retransmission is enabled. */
    bool m_enable_retransmission;

    /** The maximum number of retransmissions to perform. */
    int m_max_retransmission_count;

    /** The size of the largest possible retransmission block. */
    std::size_t m_max_retransmission_block;

    /** Constructs a TmxIpServiceAccessConfiguration with default values. */
    TmxIpServiceAccessConfiguration();
  };

  /**
   * Produces StampMessages received from a TMX Information Processor
   * service.
   * @param <F> The type of Channel receiving the venue data feed.
   * @param <C> The type of Channel used to send retransmission requests.
   * @param <S> The type of Channel used to receive retransmission messages.
   */
  template<typename F, typename C, typename S>
  class TmxIpServiceAccessClient {
    public:

      /** The type of channel receiving the venue data feed. */
      using Channel = Beam::dereference_t<F>;

      /** The type of Channel used to send retransmission requests. */
      using RetransmissionClientChannel = Beam::dereference_t<C>;

      /** The type of Channel used to receive retransmission messages. */
      using RetransmissionServerChannel = Beam::dereference_t<S>;

      /**
       * The type of function used to build instances of the
       * RetransmissionClientChannel.
       * @param channel Stores the Channel to build.
       */
      using RetransmissionClientChannelBuilder = std::function<void (
        Beam::Out<std::optional<RetransmissionClientChannel>> channel)>;

      /**
       * Constructs a TmxIpServiceAccessClient.
       * @param config The configuration to use.
       * @param channel The Channel receiving the venue data feed.
       * @param retransmission_client_channel_builder Builds instances of the
       *        Channel used to send retransmission requests.
       * @param retransmission_server_channel The Channel receiving
       *        retransmission messages.
       */
      template<typename FF, typename SF>
      TmxIpServiceAccessClient(TmxIpServiceAccessConfiguration config,
        FF&& channel, RetransmissionClientChannelBuilder
          retransmission_client_channel_builder,
        SF&& retransmission_server_channel);

      ~TmxIpServiceAccessClient();

      /** Reads the next message from the feed. */
      StampMessage read();

      void close();

    private:
      struct BufferEntry {
        Beam::SharedBuffer m_buffer;
        std::uint32_t m_sequence_number;

        BufferEntry(Beam::SharedBuffer buffer, std::uint32_t sequence_number);
      };
      TmxIpServiceAccessConfiguration m_config;
      Beam::local_ptr_t<F> m_feed_channel;
      RetransmissionClientChannelBuilder
        m_retransmission_client_channel_builder;
      Beam::local_ptr_t<S> m_retransmission_server_channel;
      int m_retransmission_count;
      std::uint32_t m_sequence_number;
      std::vector<Beam::SharedBuffer> m_buffers;
      std::deque<BufferEntry> m_pending_buffers;
      Beam::OpenState m_open_state;

      static void build_retransmission_request_buffer(
        Beam::Out<Beam::SharedBuffer> buffer, std::size_t start_sequence_number,
        std::size_t end_sequence_number);
      TmxIpServiceAccessClient(const TmxIpServiceAccessClient&) = delete;
      TmxIpServiceAccessClient& operator =(
        const TmxIpServiceAccessClient&) = delete;
      void add_pending_buffer(
        Beam::SharedBuffer buffer, std::size_t sequence_number);
      void send_retransmission_request(
        std::size_t start_sequence_number, std::size_t end_sequence_number);
      void read_retransmission_reponse(
        std::size_t start_sequence_number, std::size_t end_sequence_number);
      void retransmit(
        std::size_t start_sequence_number, std::size_t end_sequence_number);
  };

  inline TmxIpServiceAccessConfiguration::TmxIpServiceAccessConfiguration()
    : m_enable_retransmission(false),
      m_max_retransmission_count(100),
      m_max_retransmission_block(20000) {}

  template<typename F, typename C, typename S>
  TmxIpServiceAccessClient<F, C, S>::BufferEntry::BufferEntry(
    Beam::SharedBuffer buffer, std::uint32_t sequence_number)
    : m_buffer(std::move(buffer)),
      m_sequence_number(sequence_number) {}

  template<typename F, typename C, typename S>
  template<typename FF, typename SF>
  TmxIpServiceAccessClient<F, C, S>::TmxIpServiceAccessClient(
      TmxIpServiceAccessConfiguration config, FF&& feedChannel,
      RetransmissionClientChannelBuilder retransmission_client_channel_builder,
      SF&& retransmission_server_channel)
      try : m_config(std::move(config)),
            m_feed_channel(std::forward<FF>(feedChannel)),
            m_retransmission_client_channel_builder(
              std::move(retransmission_client_channel_builder)),
            m_retransmission_server_channel(
              std::forward<SF>(retransmission_server_channel)),
            m_retransmission_count(0),
            m_sequence_number(0) {
  } catch(const std::exception&) {
    std::throw_with_nested(Beam::ConnectException(
      "Failed to initialize the TMX IP service access client."));
  }

  template<typename F, typename C, typename S>
  TmxIpServiceAccessClient<F, C, S>::~TmxIpServiceAccessClient() {
    close();
  }

  template<typename F, typename C, typename S>
  StampMessage TmxIpServiceAccessClient<F, C, S>::read() {
    static const auto HEARTBEAT_MESSAGE_TYPE = Beam::FixedString<2>("V ");
    return Beam::try_or_nest([&] {
      if(m_config.m_enable_retransmission) {
        auto retransmission_buffer = Beam::SharedBuffer();
        while(m_retransmission_server_channel->get_reader().poll()) {
          reset(retransmission_buffer);
          m_retransmission_server_channel->get_reader().read(
            Beam::out(retransmission_buffer));
        }
      }
      auto buffer_index = std::size_t(0);
      while(true) {
        if(m_buffers.size() <= buffer_index) {
          m_buffers.emplace_back();
        }
        auto& buffer = m_buffers[buffer_index];
        reset(buffer);
        if(m_pending_buffers.empty()) {
          m_feed_channel->get_reader().read(Beam::out(buffer));
        } else {
          buffer = std::move(m_pending_buffers.front().m_buffer);
          m_pending_buffers.pop_front();
        }
        auto packet = StampPacket::parse(buffer.get_data(), buffer.get_size());
        if(packet.m_header.m_message_type == HEARTBEAT_MESSAGE_TYPE) {
          continue;
        }
        if(m_sequence_number == 0) {
          if(packet.m_header.m_continuation_indicator ==
              ContinuationIndicator::STAND_ALONE ||
                packet.m_header.m_continuation_indicator ==
                  ContinuationIndicator::SPANNING) {
            m_sequence_number = packet.m_header.m_sequence_number;
          } else {
            continue;
          }
        } else if(packet.m_header.m_sequence_number == m_sequence_number + 1) {
          ++m_sequence_number;
        } else if(packet.m_header.m_sequence_number <= m_sequence_number) {
          continue;
        } else {
          std::cout << "Dropped packets: " << m_sequence_number + 1 << " - " <<
            packet.m_header.m_sequence_number - 1 << std::endl;
          add_pending_buffer(buffer, packet.m_header.m_sequence_number);
          if(m_config.m_enable_retransmission) {
            try {
              retransmit(m_sequence_number + 1,
                packet.m_header.m_sequence_number - 1);
            } catch(const std::exception&) {
              m_sequence_number = 0;
              buffer_index = 0;
            }
          } else {
            m_sequence_number = 0;
            buffer_index = 0;
          }
          continue;
        }
        if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::STAND_ALONE) {
          auto message = StampMessage(packet.m_header,
            packet.m_message, packet.m_message_size);
          return message;
        } else if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::SPANNING) {
          buffer_index = 1;
        } else if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::SPANNING_CONTINUATION) {
          ++buffer_index;
        } else if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::CONTINUATION) {
          buffer_index = 0;
        }
      }
    }, Beam::IOException("Unable to read STAMP message."));
  }

  template<typename F, typename C, typename S>
  void TmxIpServiceAccessClient<F, C, S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    if(m_config.m_enable_retransmission) {
      m_retransmission_server_channel->get_connection().close();
    }
    m_feed_channel->get_connection().close();
    m_buffers.clear();
    m_open_state.close();
  }

  template<typename F, typename C, typename S>
  void TmxIpServiceAccessClient<F, C, S>::build_retransmission_request_buffer(
      Beam::Out<Beam::SharedBuffer> buffer, std::size_t start_sequence_number,
      std::size_t end_sequence_number) {
    constexpr auto SEQUENCE_NUMBER_SIZE = std::size_t(9);
    append(*buffer, "SEQN", 4);
    auto message_start_number =
      boost::lexical_cast<std::string>(start_sequence_number);
    while(message_start_number.size() < SEQUENCE_NUMBER_SIZE) {
      message_start_number.insert(message_start_number.begin(), '0');
    }
    append(*buffer, message_start_number.c_str(), SEQUENCE_NUMBER_SIZE);
    auto message_end_number =
      boost::lexical_cast<std::string>(end_sequence_number);
    while(message_end_number.size() < SEQUENCE_NUMBER_SIZE) {
      message_end_number.insert(message_end_number.begin(), '0');
    }
    append(*buffer, message_end_number.c_str(), SEQUENCE_NUMBER_SIZE);
    append(*buffer, '\n');
  }

  template<typename F, typename C, typename S>
  void TmxIpServiceAccessClient<F, C, S>::add_pending_buffer(
      Beam::SharedBuffer buffer, std::size_t sequence_number) {
    auto entry = BufferEntry(std::move(buffer), sequence_number);
    auto pending_buffer_iterator = std::lower_bound(
      m_pending_buffers.begin(), m_pending_buffers.end(), entry,
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_sequence_number < rhs.m_sequence_number;
      });
    if(pending_buffer_iterator == m_pending_buffers.end() ||
        pending_buffer_iterator->m_sequence_number != sequence_number) {
      m_pending_buffers.insert(pending_buffer_iterator, entry);
    }
  }

  template<typename F, typename C, typename S>
  void TmxIpServiceAccessClient<F, C, S>::send_retransmission_request(
      std::size_t start_sequence_number, std::size_t end_sequence_number) {
    constexpr auto RETRANSMISSION_RESPONSE_SIZE = std::size_t(151);
    auto retransmission_request_buffer = Beam::SharedBuffer();
    build_retransmission_request_buffer(
      Beam::out(retransmission_request_buffer), start_sequence_number,
      end_sequence_number);
    auto retransmission_client_channel =
      std::optional<RetransmissionClientChannel>();
    m_retransmission_client_channel_builder(
      Beam::out(retransmission_client_channel));
    retransmission_client_channel->get_writer().write(
      retransmission_request_buffer);
    auto retransmission_response_buffer = Beam::SharedBuffer();
    while(retransmission_response_buffer.get_size() <
        RETRANSMISSION_RESPONSE_SIZE) {
      retransmission_client_channel->get_reader().read(
        Beam::out(retransmission_response_buffer));
    }
    reset(retransmission_response_buffer);
    try {
      retransmission_client_channel->get_reader().read(
        Beam::out(retransmission_response_buffer));
    } catch(const std::exception&) {}
  }

  template<typename F, typename C, typename S>
  void TmxIpServiceAccessClient<F, C, S>::read_retransmission_reponse(
      std::size_t start_sequence_number, std::size_t end_sequence_number) {
    auto packet = StampPacket();
    auto retransmission_buffer = Beam::SharedBuffer();
    while(m_retransmission_server_channel->get_reader().poll()) {
      reset(retransmission_buffer);
      m_retransmission_server_channel->get_reader().read(
        Beam::out(retransmission_buffer));
      auto packet = StampPacket::parse(
        retransmission_buffer.get_data(), retransmission_buffer.get_size());
      if(packet.m_header.m_sequence_number < start_sequence_number ||
          packet.m_header.m_sequence_number > end_sequence_number) {
        continue;
      }
      std::cout << "Recovered: " << packet.m_header.m_sequence_number <<
        std::endl;
      add_pending_buffer(
        retransmission_buffer, packet.m_header.m_sequence_number);
    }
  }

  template<typename F, typename C, typename S>
  void TmxIpServiceAccessClient<F, C, S>::retransmit(
      std::size_t start_sequence_number, std::size_t end_sequence_number) {
    while(start_sequence_number <= end_sequence_number) {
      if(m_retransmission_count > m_config.m_max_retransmission_count) {
        boost::throw_with_location(
          std::runtime_error("Too many retransmissions"));
      }
      auto end_block = std::min(end_sequence_number,
        start_sequence_number + m_config.m_max_retransmission_block - 1);
      ++m_retransmission_count;
      send_retransmission_request(start_sequence_number, end_block);
      read_retransmission_reponse(start_sequence_number, end_block);
      start_sequence_number = end_block + 1;
    }
  }
}

#endif
