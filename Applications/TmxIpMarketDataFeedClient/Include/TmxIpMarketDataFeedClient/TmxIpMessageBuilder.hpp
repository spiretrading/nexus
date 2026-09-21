#ifndef TMX_IP_MESSAGE_BUILDER_HPP
#define TMX_IP_MESSAGE_BUILDER_HPP
#include <string>
#include <utility>
#include <Beam/IO/SharedBuffer.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpPacket.hpp"

namespace Nexus {

  /** Reconstructs message bodies from one sequenced TMX IP stream. */
  class TmxIpMessageBuilder {
    public:

      /**
       * Adds a packet, discarding any assembly it cannot continue.
       * @param packet The next parsed packet after sequencing and recovery.
       * @return An owned complete message body, or none while incomplete.
       */
      boost::optional<Beam::SharedBuffer> add(const TmxIpPacket& packet);

      /** Returns whether a message is awaiting continuation packets. */
      bool has_pending_message() const;

      /** Discards an incomplete message. */
      void reset();

    private:
      struct Assembly {
        std::uint32_t m_sequence;
        std::string m_service;
        char m_retransmission;
        char m_exchange;
        Beam::SharedBuffer m_payload;
      };
      boost::optional<Assembly> m_assembly;
  };

  inline boost::optional<Beam::SharedBuffer> TmxIpMessageBuilder::add(
      const TmxIpPacket& packet) {
    auto& header = packet.m_header;
    if(!header.m_sequence) {
      return boost::none;
    }
    if(header.m_continuation == TmxIpHeader::Continuation::NONE) {
      reset();
      return Beam::SharedBuffer(
        packet.m_payload.data(), packet.m_payload.size());
    }
    if(header.m_continuation == TmxIpHeader::Continuation::FIRST) {
      m_assembly.emplace(*header.m_sequence, std::string(header.m_service),
        header.m_retransmission, header.m_exchange,
        Beam::SharedBuffer(packet.m_payload.data(), packet.m_payload.size()));
      return boost::none;
    }
    if(!m_assembly) {
      return boost::none;
    }
    constexpr auto MAXIMUM_SEQUENCE = 999999999;
    auto sequence = m_assembly->m_sequence + 1;
    if(sequence > MAXIMUM_SEQUENCE) {
      sequence = 1;
    }
    auto is_continuation = header.m_sequence == sequence &&
      header.m_service == m_assembly->m_service &&
      header.m_retransmission == m_assembly->m_retransmission &&
      header.m_exchange == m_assembly->m_exchange;
    if(!is_continuation) {
      reset();
      return boost::none;
    }
    Beam::append(m_assembly->m_payload, packet.m_payload);
    m_assembly->m_sequence = sequence;
    if(header.m_continuation == TmxIpHeader::Continuation::MIDDLE) {
      return boost::none;
    }
    auto payload = std::move(m_assembly->m_payload);
    reset();
    return payload;
  }

  inline bool TmxIpMessageBuilder::has_pending_message() const {
    return m_assembly.has_value();
  }

  inline void TmxIpMessageBuilder::reset() {
    m_assembly = boost::none;
  }
}

#endif
