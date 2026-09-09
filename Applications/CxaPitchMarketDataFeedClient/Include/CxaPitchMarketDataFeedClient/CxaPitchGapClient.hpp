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
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/lock_guard.hpp>
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
    { t.is_recoverable(std::declval<const CxaPitchGap&>(),
        std::declval<std::uint32_t>()) } -> std::same_as<bool>;
    { t.get_responses() } -> std::convertible_to<
      const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Requests the retransmission of missing messages from a CXA PITCH gap
   * request proxy, within the limits that the proxy imposes.
   * @param <S> The type of session connected to the gap request proxy.
   * @param <T> The type of Timer measuring the delay between connections.
   */
  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  class CxaPitchGapClient {
    public:

      /** The type of session connected to the gap request proxy. */
      using Session = S;

      /** The type of timer measuring the delay between connections. */
      using Timer = Beam::dereference_t<T>;

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
       * @param connection_builder The function used to connect a session to the
       *        gap request proxy.
       * @param timer The timer measuring the delay between connections.
       */
      template<Beam::Initializes<T> TF>
      CxaPitchGapClient(
        std::function<std::shared_ptr<Session> ()> connection_builder,
        TF&& timer);

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

      /**
       * Returns whether the proxy is able to retransmit a range of messages.
       * @param gap The range of missing messages.
       * @param live The most recent sequence received from the feeds.
       * @return Whether the proxy is able to retransmit the <i>gap</i>.
       */
      bool is_recoverable(const CxaPitchGap& gap, std::uint32_t live) const;

      /** Returns the queue of responses sent by the gap request proxy. */
      const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&
        get_responses() const;

      /** Closes the connection to the gap request proxy. */
      void close();

    private:
      mutable Beam::Mutex m_mutex;
      std::function<std::shared_ptr<Session> ()> m_connection_builder;
      std::shared_ptr<Session> m_session;
      Beam::local_ptr_t<T> m_timer;
      bool m_is_connected;
      std::shared_ptr<Beam::Queue<CxaPitchGapResponse>> m_responses;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_timer_queue;
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
      bool reconnect();
      void read_loop();
  };

  template<typename S, typename TF>
  CxaPitchGapClient(std::function<std::shared_ptr<S> ()>, TF&&) ->
    CxaPitchGapClient<S, std::remove_cvref_t<TF>>;

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  CxaPitchGapClient<S, T>::CxaPitchGapClient(
      std::function<std::shared_ptr<Session> ()> connection_builder, TF&& timer)
      try : m_connection_builder(std::move(connection_builder)),
            m_session(m_connection_builder()),
            m_timer(std::forward<TF>(timer)),
            m_is_connected(true),
            m_responses(std::make_shared<Beam::Queue<CxaPitchGapResponse>>()),
            m_timer_queue(std::make_shared<Beam::Queue<Beam::Timer::Result>>()),
            m_second_count(0),
            m_minute_count(0),
            m_day_count(0) {
    m_timer->get_publisher().monitor(m_timer_queue);
    m_read_loop =
      Beam::spawn(std::bind_front(&CxaPitchGapClient::read_loop, this));
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(Beam::ConnectException(
      "Failed to initialize the CXA PITCH gap client."));
  }

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  CxaPitchGapClient<S, T>::~CxaPitchGapClient() {
    close();
  }

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  std::uint32_t CxaPitchGapClient<S, T>::request(std::uint8_t unit,
      const CxaPitchGap& gap, std::uint32_t live,
      boost::posix_time::ptime timestamp) {
    if(!is_recoverable(gap, live)) {
      return 0;
    }
    auto requested = std::uint32_t(0);
    while(requested != gap.m_count) {
      auto count = std::min(gap.m_count - requested, MAXIMUM_COUNT);
      auto session = [&] () -> std::shared_ptr<Session> {
        auto lock = boost::lock_guard(m_mutex);
        renew(timestamp);
        if(!m_is_connected || m_second_count == SECOND_LIMIT ||
            m_minute_count == MINUTE_LIMIT || m_day_count == DAY_LIMIT) {
          return nullptr;
        }
        ++m_second_count;
        ++m_minute_count;
        ++m_day_count;
        return m_session;
      }();
      if(!session) {
        break;
      }
      auto message = CxaPitchGapRequest();
      message.m_unit = unit;
      message.m_sequence = gap.m_sequence + requested;
      message.m_count = static_cast<std::uint16_t>(count);
      try {
        session->write(message);
      } catch(const std::exception&) {
        {
          auto lock = boost::lock_guard(m_mutex);
          m_is_connected = false;
        }
        session->close();
        break;
      }
      requested += count;
    }
    return requested;
  }

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  bool CxaPitchGapClient<S, T>::is_recoverable(
      const CxaPitchGap& gap, std::uint32_t live) const {
    return live <= gap.m_sequence || live - gap.m_sequence <= MAXIMUM_RANGE;
  }

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  const std::shared_ptr<Beam::Queue<CxaPitchGapResponse>>&
      CxaPitchGapClient<S, T>::get_responses() const {
    return m_responses;
  }

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchGapClient<S, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_timer->cancel();
    m_timer_queue->close();
    auto session = [&] {
      auto lock = boost::lock_guard(m_mutex);
      m_is_connected = false;
      return m_session;
    }();
    session->close();
    m_read_loop.wait();
    m_responses->close();
    m_open_state.close();
  }

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchGapClient<S, T>::renew(boost::posix_time::ptime timestamp) {
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

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  bool CxaPitchGapClient<S, T>::reconnect() {
    while(m_open_state.is_open()) {
      m_timer->start();
      try {
        if(m_timer_queue->pop() != Beam::Timer::Result::EXPIRED) {
          return false;
        }
      } catch(const std::exception&) {
        return false;
      }
      if(!m_open_state.is_open()) {
        return false;
      }
      try {
        auto session = m_connection_builder();
        {
          auto lock = boost::lock_guard(m_mutex);
          if(m_open_state.is_open()) {
            m_session = session;
            m_is_connected = true;
            return true;
          }
        }
        session->close();
        return false;
      } catch(const std::exception&) {}
    }
    return false;
  }

  template<IsCxaPitchSessionClient S, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchGapClient<S, T>::read_loop() {
    while(true) {
      auto session = [&] {
        auto lock = boost::lock_guard(m_mutex);
        return m_session;
      }();
      try {
        while(true) {
          auto message = session->read();
          if(message.m_type == CxaPitchGapResponse::TYPE) {
            m_responses->push(CxaPitchGapResponse::parse(message));
          }
        }
      } catch(const std::exception&) {}
      {
        auto lock = boost::lock_guard(m_mutex);
        m_is_connected = false;
      }
      if(!m_open_state.is_open() || !reconnect()) {
        break;
      }
    }
  }
}

#endif
