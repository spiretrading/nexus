#ifndef CXA_PITCH_GAP_CLIENT_HPP
#define CXA_PITCH_GAP_CLIENT_HPP
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchSequencer.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionMessages.hpp"

namespace Nexus {

  /** Concept satisfied by types requesting the retransmission of messages. */
  template<typename T>
  concept IsCxaPitchGapClient = requires(T& t) {
    { t.request(std::declval<std::uint8_t>(),
        std::declval<const CxaPitchGap&>(), std::declval<std::uint32_t>(),
        std::declval<boost::posix_time::ptime>()) } ->
          std::same_as<std::uint32_t>;
    { t.get_responses() } -> std::convertible_to<
      const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Requests the retransmission of missing messages from a CXA PITCH gap
   * request proxy, within the limits that the proxy imposes.
   * @param <S> The type of session connected to the gap request proxy.
   */
  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  class CxaPitchGapClient {
    public:

      /** The type of session connected to the gap request proxy. */
      using Session = Beam::dereference_t<S>;

      /** The most messages that a single request may ask for. */
      static constexpr auto MAXIMUM_COUNT = std::uint32_t(100);

      /** The most requests permitted within a clock second. */
      static constexpr auto SECOND_LIMIT = 320;

      /** The most requests permitted within a clock minute. */
      static constexpr auto MINUTE_LIMIT = 1500;

      /** The most requests permitted within a day. */
      static constexpr auto DAY_LIMIT = 100000;

      /** The furthest that a request may lag the live sequence. */
      static constexpr auto MAXIMUM_RANGE = std::uint32_t(1000000);

      /**
       * Constructs a CxaPitchGapClient.
       * @param session The session connected to the gap request proxy.
       */
      template<Beam::Initializes<S> SF>
      explicit CxaPitchGapClient(SF&& session);

      ~CxaPitchGapClient();

      /**
       * Requests the retransmission of a range of missing messages.
       * @param unit The unit that the missing messages belong to.
       * @param gap The range of missing messages.
       * @param live The most recent sequence received from the feeds.
       * @param timestamp The time that the request is made.
       * @return The number of messages that were requested.
       */
      std::uint32_t request(std::uint8_t unit, const CxaPitchGap& gap,
        std::uint32_t live, boost::posix_time::ptime timestamp);

      /** Returns the queue of responses sent by the gap request proxy. */
      const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&
        get_responses() const;

      /** Closes the connection to the gap request proxy. */
      void close();

    private:
      Beam::local_ptr_t<S> m_session;
      std::shared_ptr<Beam::Queue<CxaPitchGapResponse>> m_responses;
      boost::posix_time::ptime m_second;
      boost::posix_time::ptime m_minute;
      boost::gregorian::date m_day;
      int m_second_count;
      int m_minute_count;
      int m_day_count;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      CxaPitchGapClient(const CxaPitchGapClient&) = delete;
      CxaPitchGapClient& operator =(const CxaPitchGapClient&) = delete;
      void renew(boost::posix_time::ptime timestamp);
      void read_loop();
  };

  template<typename SF>
  CxaPitchGapClient(SF&&) -> CxaPitchGapClient<std::remove_cvref_t<SF>>;

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  template<Beam::Initializes<S> SF>
  CxaPitchGapClient<S>::CxaPitchGapClient(SF&& session)
      try : m_session(std::forward<SF>(session)),
            m_responses(std::make_shared<Beam::Queue<CxaPitchGapResponse>>()),
            m_second_count(0),
            m_minute_count(0),
            m_day_count(0) {
    m_read_loop =
      Beam::spawn(std::bind_front(&CxaPitchGapClient::read_loop, this));
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(Beam::ConnectException(
      "Failed to initialize the CXA PITCH gap client."));
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  CxaPitchGapClient<S>::~CxaPitchGapClient() {
    close();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  std::uint32_t CxaPitchGapClient<S>::request(std::uint8_t unit,
      const CxaPitchGap& gap, std::uint32_t live,
      boost::posix_time::ptime timestamp) {
    renew(timestamp);
    if(live > gap.m_sequence && live - gap.m_sequence > MAXIMUM_RANGE) {
      return 0;
    }
    auto requested = std::uint32_t(0);
    while(requested != gap.m_count) {
      if(m_second_count == SECOND_LIMIT || m_minute_count == MINUTE_LIMIT ||
          m_day_count == DAY_LIMIT) {
        break;
      }
      auto count = std::min(gap.m_count - requested, MAXIMUM_COUNT);
      auto message = CxaPitchGapRequest();
      message.m_unit = unit;
      message.m_sequence = gap.m_sequence + requested;
      message.m_count = static_cast<std::uint16_t>(count);
      m_session->write(message);
      ++m_second_count;
      ++m_minute_count;
      ++m_day_count;
      requested += count;
    }
    return requested;
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&
      CxaPitchGapClient<S>::get_responses() const {
    return m_responses;
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  void CxaPitchGapClient<S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_session->close();
    m_responses->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  void CxaPitchGapClient<S>::renew(boost::posix_time::ptime timestamp) {
    auto time = timestamp.time_of_day();
    auto second = boost::posix_time::ptime(
      timestamp.date(), boost::posix_time::seconds(time.total_seconds()));
    if(second != m_second) {
      m_second = second;
      m_second_count = 0;
    }
    auto minute = boost::posix_time::ptime(
      timestamp.date(), boost::posix_time::minutes(time.total_seconds() / 60));
    if(minute != m_minute) {
      m_minute = minute;
      m_minute_count = 0;
    }
    if(timestamp.date() != m_day) {
      m_day = timestamp.date();
      m_day_count = 0;
    }
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  void CxaPitchGapClient<S>::read_loop() {
    try {
      while(true) {
        auto message = m_session->read();
        if(message.m_type == CxaPitchGapResponse::TYPE) {
          m_responses->push(CxaPitchGapResponse::parse(message));
        }
      }
    } catch(const std::exception&) {
      m_responses->close(std::current_exception());
    }
  }
}

#endif
