#ifndef TMX_IP_CLIENT_HPP
#define TMX_IP_CLIENT_HPP
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpMessageBuilder.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpRecoveryClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpSequencer.hpp"

namespace Nexus {

  /** Concept satisfied by clients delivering ordered STAMP messages. */
  template<typename T>
  concept IsTmxIpClient = requires(T& t) {
    { t.read() } -> std::same_as<StampMessage>;
    { t.read(std::declval<Beam::Out<std::uint64_t>>()) } ->
      std::same_as<StampMessage>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Delivers complete, ordered messages from one TMX IP stream.
   * @tparam P The live protocol client type.
   * @tparam G The recovery client type.
   * @tparam R The time client type.
   * @tparam T The timer driving feed expiry, gap deadlines and retries.
   */
  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class TmxIpClient {
    public:

      /** The live protocol client type. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The recovery client type. */
      using RecoveryClient = Beam::dereference_t<G>;

      /** The source of the current time. */
      using TimeClient = Beam::dereference_t<R>;

      /** The timer driving feed expiry, gap deadlines and retries. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs a TMX IP client.
       * @param time_zone The transport's time zone.
       * @param rollover_time The daily session boundary in local time.
       * @param feed_timeout How long a stalled feed delays gap confirmation.
       * @param gap_timeout How long to wait before abandoning recovery.
       * @param feed_clients Receives copies of the live stream.
       * @param recovery_client Requests and receives missing packets, if set.
       * @param time_client Supplies the current time.
       * @param timer Drives feed expiry, gap deadlines and recovery retries.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<G> GF,
        Beam::Initializes<R> RF, Beam::Initializes<T> TF>
      TmxIpClient(boost::local_time::time_zone_ptr time_zone,
        boost::posix_time::time_duration rollover_time,
        boost::posix_time::time_duration feed_timeout,
        boost::posix_time::time_duration gap_timeout,
        std::vector<PF> feed_clients, boost::optional<GF> recovery_client,
        RF&& time_client, TF&& timer);

      ~TmxIpClient();

      /**
       * Reads the next complete STAMP message.
       * The returned views remain valid until the next read or destruction.
       * Throws when all live feeds fail or a message is malformed.
       */
      StampMessage read();

      /** Reads a message and its transport session identifier. */
      StampMessage read(Beam::Out<std::uint64_t> session);

      void close();

    private:
      enum class RequestState {
        READY,
        PENDING,
        DEFERRED
      };
      static constexpr auto MAXIMUM_SEQUENCE = std::uint32_t(999999999);
      struct Feed {
        boost::optional<std::uint32_t> m_position;
        boost::posix_time::ptime m_timestamp;
        bool m_is_closed = false;
      };
      struct State {
        TmxIpSequencer m_sequencer;
        TmxIpMessageBuilder m_builder;
        std::vector<Feed> m_feeds;
        std::map<boost::posix_time::ptime, boost::optional<std::uint32_t>>
          m_retired_sessions;
        std::string m_recovery_error;
        boost::optional<TmxIpGap> m_gap;
        boost::posix_time::ptime m_gap_timestamp;
        RequestState m_request_state = RequestState::READY;
        bool m_is_finished = false;
        std::uint64_t m_session = 0;
      };
      struct Message {
        Beam::SharedBuffer m_buffer;
        StampMessage m_message;
        std::uint64_t m_session;
      };
      boost::local_time::time_zone_ptr m_time_zone;
      boost::posix_time::time_duration m_rollover_time;
      boost::posix_time::time_duration m_feed_timeout;
      boost::posix_time::time_duration m_gap_timeout;
      boost::posix_time::ptime m_session_start;
      boost::posix_time::ptime m_session_end;
      std::vector<Beam::local_ptr_t<P>> m_feed_clients;
      boost::optional<Beam::local_ptr_t<G>> m_recovery_client;
      Beam::local_ptr_t<R> m_time_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Sync<State> m_state;
      Beam::Queue<Message> m_messages;
      Beam::Queue<std::uint64_t> m_requests;
      Beam::SharedBuffer m_payload;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandlerGroup m_routines;
      Beam::OpenState m_open_state;

      TmxIpClient(const TmxIpClient&) = delete;
      TmxIpClient& operator =(const TmxIpClient&) = delete;
      static std::uint32_t distance(std::uint32_t first, std::uint32_t last);
      std::pair<boost::posix_time::ptime, boost::posix_time::ptime> get_session(
        boost::posix_time::ptime timestamp) const;
      void update_session(State& state);
      void discard(State& state, const TmxIpPacket& packet,
        boost::posix_time::ptime timestamp);
      void add(State& state, const TmxIpPacket& packet);
      void flush(State& state);
      void drop(State& state, const TmxIpGap& gap, std::string_view reason);
      void update(State& state, std::size_t feed, const TmxIpPacket& packet);
      boost::optional<TmxIpGap> get_gap(const State& state) const;
      void request(State& state);
      void fail(const std::exception_ptr& error);
      void feed_loop(std::size_t feed);
      void recovery_loop();
      void request_loop();
      void on_timer(typename Timer::Result result);
  };

  template<typename PF, typename GF, typename RF, typename TF>
  TmxIpClient(boost::local_time::time_zone_ptr,
    boost::posix_time::time_duration, boost::posix_time::time_duration,
    boost::posix_time::time_duration, std::vector<PF>,
    boost::optional<GF>, RF&&, TF&&) -> TmxIpClient<PF, GF,
      std::remove_cvref_t<RF>, std::remove_cvref_t<TF>>;

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<P> PF, Beam::Initializes<G> GF,
    Beam::Initializes<R> RF, Beam::Initializes<T> TF>
  TmxIpClient<P, G, R, T>::TmxIpClient(
      boost::local_time::time_zone_ptr time_zone,
      boost::posix_time::time_duration rollover_time,
      boost::posix_time::time_duration feed_timeout,
      boost::posix_time::time_duration gap_timeout,
      std::vector<PF> feed_clients, boost::optional<GF> recovery_client,
      RF&& time_client, TF&& timer)
      : m_time_zone(std::move(time_zone)),
        m_rollover_time(rollover_time),
        m_feed_timeout(feed_timeout),
        m_gap_timeout(gap_timeout),
        m_time_client(std::forward<RF>(time_client)),
        m_timer(std::forward<TF>(timer)) {
    try {
      if(!m_time_zone || rollover_time.is_special() ||
          rollover_time < boost::posix_time::seconds(0) ||
          rollover_time >= boost::posix_time::hours(24) ||
          feed_clients.empty() || feed_timeout.is_special() ||
          feed_timeout <= boost::posix_time::seconds(0) ||
          gap_timeout.is_special() ||
          gap_timeout <= boost::posix_time::seconds(0)) {
        boost::throw_with_location(
          std::invalid_argument("Invalid TMX IP feed configuration."));
      }
      for(auto& client : feed_clients) {
        m_feed_clients.emplace_back(std::move(client));
      }
      if(recovery_client) {
        m_recovery_client.emplace(std::move(*recovery_client));
      }
      auto session = get_session(m_time_client->get_time());
      m_session_start = session.first;
      m_session_end = session.second;
      Beam::with(m_state, [&] (auto& state) {
        state.m_feeds.resize(m_feed_clients.size());
        for(auto& feed : state.m_feeds) {
          feed.m_timestamp = m_time_client->get_time();
        }
      });
      m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
        std::bind_front(&TmxIpClient::on_timer, this)));
      m_timer->start();
      for(auto i = std::size_t(0); i != m_feed_clients.size(); ++i) {
        m_routines.spawn(std::bind_front(&TmxIpClient::feed_loop, this, i));
      }
      if(m_recovery_client) {
        m_routines.spawn(std::bind_front(&TmxIpClient::recovery_loop, this));
        m_routines.spawn(std::bind_front(&TmxIpClient::request_loop, this));
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  TmxIpClient<P, G, R, T>::~TmxIpClient() {
    close();
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  StampMessage TmxIpClient<P, G, R, T>::read() {
    auto session = std::uint64_t();
    return read(Beam::out(session));
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  StampMessage TmxIpClient<P, G, R, T>::read(
      Beam::Out<std::uint64_t> session) {
    auto message = m_messages.pop();
    *session = message.m_session;
    m_payload = std::move(message.m_buffer);
    return message.m_message;
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    fail(std::make_exception_ptr(Beam::EndOfFileException()));
    for(auto& client : m_feed_clients) {
      client->close();
    }
    if(m_recovery_client) {
      (*m_recovery_client)->close();
    }
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_routines.wait();
    m_open_state.close();
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  std::uint32_t TmxIpClient<P, G, R, T>::distance(
      std::uint32_t first, std::uint32_t last) {
    if(last >= first) {
      return last - first;
    }
    return MAXIMUM_SEQUENCE - first + last;
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  std::pair<boost::posix_time::ptime, boost::posix_time::ptime>
      TmxIpClient<P, G, R, T>::get_session(
        boost::posix_time::ptime timestamp) const {
    auto local =
      boost::local_time::local_date_time(timestamp, m_time_zone).local_time();
    auto resolve = [&] (boost::gregorian::date date) {
      auto candidate = boost::posix_time::ptime(date, m_rollover_time) -
        m_time_zone->base_utc_offset();
      if(!m_time_zone->has_dst()) {
        return candidate;
      }
      auto daylight = candidate - m_time_zone->dst_offset();
      auto daylight_local = boost::local_time::local_date_time(
        daylight, m_time_zone).local_time();
      auto standard_local = boost::local_time::local_date_time(
        candidate, m_time_zone).local_time();
      auto expected = boost::posix_time::ptime(date, m_rollover_time);
      if(daylight_local == expected && standard_local == expected) {
        return std::min(candidate, daylight);
      }
      if(daylight_local == expected) {
        return daylight;
      }
      if(standard_local == expected) {
        return candidate;
      }
      return std::max(candidate, daylight);
    };
    auto date = local.date();
    auto start = resolve(date);
    if(start > timestamp) {
      date -= boost::gregorian::days(1);
      start = resolve(date);
    }
    return std::pair(start, resolve(date + boost::gregorian::days(1)));
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::update_session(State& state) {
    auto timestamp = m_time_client->get_time();
    if(state.m_is_finished || timestamp < m_session_end) {
      return;
    }
    auto sequence = state.m_sequencer.get_sequence();
    auto count = std::uint64_t();
    while(true) {
      if(state.m_sequencer.read()) {
        ++count;
      } else if(auto gap = state.m_sequencer.get_gap()) {
        count += gap->m_count;
        state.m_sequencer.skip(gap->m_count);
      } else {
        break;
      }
    }
    auto out = std::stringstream();
    if(count != 0) {
      out << "(dropped " << timestamp << ' ' << *sequence << ' ' << count <<
        " session_reset)\n";
    }
    if(state.m_builder.has_pending_message()) {
      out << "(dropped " << timestamp << " incomplete_message session_reset)\n";
    }
    if(!out.str().empty()) {
      std::cout << out.str() << std::flush;
    }
    state.m_retired_sessions[m_session_start] =
      state.m_sequencer.get_sequence();
    auto session = get_session(timestamp);
    m_session_start = session.first;
    m_session_end = session.second;
    ++state.m_session;
    state.m_sequencer.reset(1);
    state.m_builder.reset();
    state.m_gap = boost::none;
    state.m_request_state = RequestState::READY;
    for(auto& feed : state.m_feeds) {
      feed.m_position = boost::none;
      feed.m_timestamp = timestamp;
    }
    if(m_recovery_client) {
      (*m_recovery_client)->reset(state.m_session);
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::discard(State& state, const TmxIpPacket& packet,
      boost::posix_time::ptime timestamp) {
    auto sequence = packet.m_header.m_sequence;
    if(is_heartbeat(packet.m_header)) {
      sequence = TmxIpHeartbeat::parse(packet).m_last_sequence;
    }
    if(!sequence || *sequence == 0) {
      return;
    }
    auto& expected = state.m_retired_sessions[get_session(timestamp).first];
    if(!expected) {
      if(!packet.m_header.m_sequence) {
        return;
      }
      expected = sequence;
    }
    auto offset = distance(*expected, *sequence);
    if(offset > MAXIMUM_SEQUENCE / 2) {
      return;
    }
    auto out = std::stringstream();
    out << "(dropped " << m_time_client->get_time() << ' ' << *expected <<
      ' ' << offset + 1 << " session_reset)\n";
    std::cout << out.str() << std::flush;
    expected = *sequence % MAXIMUM_SEQUENCE + 1;
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::add(State& state, const TmxIpPacket& packet) {
    if(state.m_is_finished) {
      return;
    }
    try {
      state.m_sequencer.add(packet);
      flush(state);
      request(state);
    } catch(const std::exception&) {
      state.m_is_finished = true;
      throw;
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::flush(State& state) {
    try {
      while(auto packet = state.m_sequencer.read()) {
        if(auto payload = state.m_builder.add(*packet)) {
          auto message = StampMessage::parse(
            std::string_view(payload->get_data(), payload->get_size()));
          m_messages.push(
            Message(std::move(*payload), message, state.m_session));
        }
      }
    } catch(const std::exception&) {
      state.m_is_finished = true;
      throw;
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::drop(
      State& state, const TmxIpGap& gap, std::string_view reason) {
    auto out = std::stringstream();
    out << "(dropped " << m_time_client->get_time() << ' ' <<
      gap.m_sequence << ' ' << gap.m_count << ' ' << reason << ")\n";
    std::cout << out.str() << std::flush;
    state.m_builder.reset();
    state.m_sequencer.skip(gap.m_count);
    state.m_gap = boost::none;
    flush(state);
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::update(
      State& state, std::size_t feed, const TmxIpPacket& packet) {
    auto sequence = packet.m_header.m_sequence;
    auto is_idle = is_heartbeat(packet.m_header);
    if(is_idle) {
      sequence = TmxIpHeartbeat::parse(packet).m_last_sequence;
    }
    if(!sequence) {
      return;
    }
    auto position = *sequence % MAXIMUM_SEQUENCE + 1;
    auto& source = state.m_feeds[feed];
    auto is_advancing = [&] {
      if(!source.m_position) {
        return true;
      }
      auto offset = distance(*source.m_position, position);
      return offset != 0 && offset <= MAXIMUM_SEQUENCE / 2;
    };
    auto is_current_heartbeat = [&] {
      return is_idle && source.m_position == position &&
        std::ranges::none_of(state.m_feeds, [&] (const auto& other) {
          if(other.m_is_closed || !other.m_position) {
            return false;
          }
          auto offset = distance(position, *other.m_position);
          return offset != 0 && offset <= MAXIMUM_SEQUENCE / 2;
        });
    };
    if(is_advancing() || is_current_heartbeat()) {
      source.m_position = position;
      source.m_timestamp = m_time_client->get_time();
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  boost::optional<TmxIpGap> TmxIpClient<P, G, R, T>::get_gap(
      const State& state) const {
    auto gap = state.m_sequencer.get_gap();
    if(!gap) {
      return {};
    }
    if(state.m_gap) {
      auto offset = distance(state.m_gap->m_sequence, gap->m_sequence);
      if(offset < state.m_gap->m_count) {
        gap->m_count = std::min(gap->m_count, state.m_gap->m_count - offset);
        return gap;
      }
    }
    auto timestamp = m_time_client->get_time();
    for(auto& feed : state.m_feeds) {
      if(feed.m_is_closed || timestamp - feed.m_timestamp > m_feed_timeout) {
        continue;
      }
      if(!feed.m_position) {
        return {};
      }
      auto count = distance(gap->m_sequence, *feed.m_position);
      if(count == 0 || count > MAXIMUM_SEQUENCE / 2) {
        return {};
      }
      gap->m_count = std::min(gap->m_count, count);
    }
    return gap;
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::request(State& state) {
    if(state.m_is_finished) {
      return;
    }
    auto timestamp = m_time_client->get_time();
    while(auto gap = get_gap(state)) {
      if(!state.m_gap || state.m_gap->m_sequence != gap->m_sequence) {
        state.m_gap = gap;
        state.m_gap_timestamp = timestamp;
      }
      auto reason = [&] () -> std::string_view {
        if(!m_recovery_client) {
          return "disabled";
        }
        if(!state.m_recovery_error.empty()) {
          return state.m_recovery_error;
        }
        if(timestamp < state.m_gap_timestamp) {
          return "clock_rollback";
        }
        if(timestamp - state.m_gap_timestamp > m_gap_timeout) {
          return "timeout";
        }
        return {};
      }();
      if(!reason.empty()) {
        drop(state, *gap, reason);
        continue;
      }
      if(state.m_request_state == RequestState::READY) {
        state.m_request_state = RequestState::PENDING;
        m_requests.push(state.m_session);
      }
      return;
    }
    state.m_gap = boost::none;
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::fail(const std::exception_ptr& error) {
    Beam::with(m_state, [&] (auto& state) {
      state.m_is_finished = true;
      m_messages.close(error);
      m_requests.close(error);
    });
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::feed_loop(std::size_t feed) {
    try {
      while(m_open_state.is_open()) {
        auto timestamp = boost::posix_time::ptime();
        auto packet = [&] () -> boost::optional<TmxIpPacket> {
          try {
            return m_feed_clients[feed]->read(Beam::out(timestamp));
          } catch(const TmxIpParserException&) {
            return {};
          }
        }();
        if(packet) {
          auto heartbeat_timestamp =
            boost::optional<boost::posix_time::ptime>();
          if(is_heartbeat(packet->m_header)) {
            try {
              heartbeat_timestamp = TmxIpHeartbeat::parse(*packet).m_timestamp;
            } catch(const TmxIpParserException&) {
              continue;
            }
          }
          Beam::with(m_state, [&] (auto& state) {
            update_session(state);
            auto is_stale = timestamp < m_session_start ||
              (heartbeat_timestamp && *heartbeat_timestamp < m_session_start);
            if(is_stale) {
              if(heartbeat_timestamp) {
                timestamp = std::min(timestamp, *heartbeat_timestamp);
              }
              discard(state, *packet, timestamp);
              return;
            }
            update(state, feed, *packet);
            add(state, *packet);
          });
        }
      }
    } catch(const std::exception&) {
      auto error = std::current_exception();
      try {
        auto is_finished = Beam::with(m_state, [&] (auto& state) {
          state.m_feeds[feed].m_is_closed = true;
          if(state.m_is_finished || std::ranges::all_of(state.m_feeds,
              [] (const auto& source) { return source.m_is_closed; })) {
            return true;
          }
          request(state);
          return false;
        });
        if(is_finished) {
          fail(error);
        }
      } catch(const std::exception&) {
        fail(std::current_exception());
      }
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::recovery_loop() {
    try {
      while(m_open_state.is_open()) {
        auto session = std::uint64_t();
        auto event = [&] () -> boost::optional<TmxIpRecoveryEvent> {
          try {
            return (*m_recovery_client)->read_event(Beam::out(session));
          } catch(const TmxIpParserException&) {
            return {};
          } catch(const std::exception& e) {
            Beam::with(m_state, [&] (auto& state) {
              state.m_recovery_error =
                "recovery_failed " + std::string(e.what());
              request(state);
            });
            (*m_recovery_client)->close();
            return {};
          }
        }();
        if(!event) {
          auto is_disabled = Beam::with(m_state, [] (const auto& state) {
            return !state.m_recovery_error.empty();
          });
          if(is_disabled) {
            return;
          }
          continue;
        }
        Beam::with(m_state, [&] (auto& state) {
          update_session(state);
          if(session != state.m_session || state.m_is_finished) {
            return;
          }
          if(auto packet = std::get_if<TmxIpPacket>(&*event)) {
            add(state, *packet);
            return;
          }
          auto& failure = std::get<TmxIpRecoveryFailure>(*event);
          while(auto gap = get_gap(state)) {
            if(gap->m_sequence < failure.m_request.m_start_sequence ||
                gap->m_sequence > failure.m_request.m_end_sequence) {
              break;
            }
            gap->m_count = std::min(gap->m_count,
              failure.m_request.m_end_sequence - gap->m_sequence + 1);
            drop(state, *gap, failure.m_reason);
          }
        });
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::request_loop() {
    try {
      while(true) {
        auto session = m_requests.pop();
        auto maximum_count = (*m_recovery_client)->get_maximum_count();
        auto range = Beam::with(m_state,
          [&] (auto& state) -> boost::optional<TmxIpRecoveryRequest> {
            update_session(state);
            if(state.m_is_finished || session != state.m_session ||
                !state.m_recovery_error.empty()) {
              return {};
            }
            auto gap = get_gap(state);
            if(!gap) {
              state.m_request_state = RequestState::READY;
              return {};
            }
            if(maximum_count == 0) {
              state.m_request_state = RequestState::DEFERRED;
              return {};
            }
            auto count = std::min(gap->m_count, maximum_count);
            return TmxIpRecoveryRequest(
              gap->m_sequence, gap->m_sequence + count - 1);
          });
        if(!range) {
          continue;
        }
        try {
          (*m_recovery_client)->request(*range, session);
        } catch(const std::exception&) {}
        Beam::with(m_state, [&] (auto& state) {
          update_session(state);
          if(session != state.m_session) {
            return;
          }
          state.m_request_state = RequestState::DEFERRED;
        });
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }

  template<typename P, typename G, typename R, typename T> requires
    IsTmxIpProtocolClient<Beam::dereference_t<P>> &&
      IsTmxIpRecoveryClient<Beam::dereference_t<G>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void TmxIpClient<P, G, R, T>::on_timer(typename Timer::Result result) {
    try {
      if(result == Timer::Result::CANCELED) {
        return;
      }
      if(result == Timer::Result::FAIL) {
        boost::throw_with_location(
          Beam::IOException("TMX IP client timer failed."));
      }
      auto is_finished = Beam::with(m_state, [&] (auto& state) {
        update_session(state);
        if(state.m_request_state == RequestState::DEFERRED) {
          state.m_request_state = RequestState::READY;
        }
        request(state);
        return state.m_is_finished;
      });
      if(!is_finished) {
        m_timer->start();
      }
    } catch(const std::exception&) {
      fail(std::current_exception());
    }
  }
}

#endif
