#ifndef ASX_TRADE_ITCH_RECOVERY_SCHEDULER_HPP
#define ASX_TRADE_ITCH_RECOVERY_SCHEDULER_HPP
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchSequencer.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Request.hpp"

namespace Nexus {

  /** Schedules retransmission requests for a partition's missing messages. */
  class AsxTradeItchRecoveryScheduler {
    public:

      /**
       * Constructs an AsxTradeItchRecoveryScheduler.
       * @param timeout How long to wait before repeating an unanswered request.
       */
      explicit AsxTradeItchRecoveryScheduler(
        boost::posix_time::time_duration timeout);

      /**
       * Prepares the next request, if a missing range is ready to be requested.
       * Call after reading available messages from the sequencer and on ticks.
       * @param sequencer The partition's sequencer.
       * @param timestamp The current local time.
       * @return The request to send, or none while waiting for a reply.
       */
      boost::optional<MoldUdp64Request> request(
        const AsxTradeItchSequencer& sequencer,
        boost::posix_time::ptime timestamp);

      /** Discards the pending request. */
      void reset();

    private:
      boost::posix_time::time_duration m_timeout;
      boost::optional<MoldUdp64Request> m_request;
      boost::posix_time::ptime m_timestamp;
  };

  inline AsxTradeItchRecoveryScheduler::AsxTradeItchRecoveryScheduler(
      boost::posix_time::time_duration timeout)
      : m_timeout(timeout) {
    if(timeout.is_special() || timeout <= boost::posix_time::seconds(0)) {
      boost::throw_with_location(
        std::invalid_argument("Recovery timeout must be positive."));
    }
  }

  inline boost::optional<MoldUdp64Request>
      AsxTradeItchRecoveryScheduler::request(
        const AsxTradeItchSequencer& sequencer,
        boost::posix_time::ptime timestamp) {
    auto gap = sequencer.get_gap();
    if(!gap) {
      reset();
      return boost::none;
    }
    auto& session = *sequencer.get_session();
    if(m_request && m_request->m_session == session &&
        m_request->m_sequence_number == gap->m_sequence &&
        timestamp >= m_timestamp && timestamp - m_timestamp < m_timeout) {
      return boost::none;
    }
    auto count = static_cast<std::uint16_t>(std::min<std::uint64_t>(
      gap->m_count, std::numeric_limits<std::uint16_t>::max()));
    m_request = MoldUdp64Request(session, gap->m_sequence, count);
    m_timestamp = timestamp;
    return m_request;
  }

  inline void AsxTradeItchRecoveryScheduler::reset() {
    m_request = boost::none;
  }
}

#endif
