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
#include "Nexus/Stamp/StampPacket.hpp"

namespace Nexus {

  /**
   * Produces fixed-width messages received from a TMX TL1 service.
   * @param <C> The type of Channel receiving the market data feed.
   */
  template<typename C>
  class TmxTl1ServiceAccessClient {
    public:

      /** The type of channel receiving the market data feed. */
      using Channel = Beam::dereference_t<C>;

      /**
       * Constructs a TmxTl1ServiceAccessClient.
       * @param channel The Channel receiving the market data feed.
       */
      template<typename CF>
      explicit TmxTl1ServiceAccessClient(CF&& channel);

      ~TmxTl1ServiceAccessClient();

      /** Reads the next message from the feed. */
      StampPacket read();

      void close();

    private:
      struct BufferEntry {
        Beam::SharedBuffer m_buffer;
        std::uint32_t m_sequence_number;

        BufferEntry(Beam::SharedBuffer buffer, std::uint32_t sequence_number);
      };
      Beam::local_ptr_t<C> m_channel;
      std::uint32_t m_sequence_number;
      std::vector<Beam::SharedBuffer> m_buffers;
      std::deque<BufferEntry> m_pending_buffers;
      Beam::OpenState m_open_state;

      TmxTl1ServiceAccessClient(const TmxTl1ServiceAccessClient&) = delete;
      TmxTl1ServiceAccessClient& operator =(
        const TmxTl1ServiceAccessClient&) = delete;
      void add_pending_buffer(
        Beam::SharedBuffer buffer, std::size_t sequence_number);
  };

  template<typename C>
  TmxTl1ServiceAccessClient<C>::BufferEntry::BufferEntry(
    Beam::SharedBuffer buffer, std::uint32_t sequence_number)
    : m_buffer(std::move(buffer)),
      m_sequence_number(sequence_number) {}

  template<typename C>
  template<typename CF>
  TmxTl1ServiceAccessClient<C>::TmxTl1ServiceAccessClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)),
          m_sequence_number(0) {
    } catch(const std::exception&) {
      std::throw_with_nested(Beam::ConnectException(
        "Failed to initialize the TMX TL1 service access client."));
    }

  template<typename C>
  TmxTl1ServiceAccessClient<C>::~TmxTl1ServiceAccessClient() {
    close();
  }

  template<typename C>
  StampPacket TmxTl1ServiceAccessClient<C>::read() {
    static const auto HEARTBEAT_MESSAGE_TYPE = Beam::FixedString<2>("V ");
    return Beam::try_or_nest([&] {
      auto buffer_index = std::size_t(0);
      while(true) {
        if(m_buffers.size() <= buffer_index) {
          m_buffers.emplace_back();
        }
        auto& buffer = m_buffers[buffer_index];
        reset(buffer);
        if(m_pending_buffers.empty()) {
          m_channel->get_reader().read(Beam::out(buffer));
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
          m_sequence_number = 0;
          buffer_index = 0;
          continue;
        }
        if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::STAND_ALONE) {
          return packet;
        } else if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::SPANNING) {
          std::cout << "Spanning" << std::endl;
          buffer_index = 1;
        } else if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::SPANNING_CONTINUATION) {
          std::cout << "Spanning Continuation" << std::endl;
          ++buffer_index;
        } else if(packet.m_header.m_continuation_indicator ==
            ContinuationIndicator::CONTINUATION) {
          std::cout << "Continuation" << std::endl;
          buffer_index = 0;
        }
      }
    }, Beam::IOException("Failed to read STAMP message."));
  }

  template<typename C>
  void TmxTl1ServiceAccessClient<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_buffers.clear();
    m_open_state.close();
  }

  template<typename C>
  void TmxTl1ServiceAccessClient<C>::add_pending_buffer(
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
}

#endif
