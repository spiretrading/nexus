#ifndef OTC_LINK_CLIENT_HPP
#define OTC_LINK_CLIENT_HPP
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <syncstream>
#include <vector>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "Nexus/Definitions/StandardTimeZones.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkProtocolClient.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkSequencer.hpp"

namespace Nexus {

  /** Concept satisfied by clients delivering ordered OTC Link messages. */
  template<typename T>
  concept IsOtcLinkClient = requires(T& t) {
    { t.read() } -> std::same_as<OtcLinkMessage>;
    { t.read(std::declval<Beam::Out<std::uint64_t>>()) } ->
      std::same_as<OtcLinkMessage>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Delivers ordered live messages from one OTC Link channel.
   * Each feed must receive reset notifications to remain in the current
   * session.
   * @tparam P The protocol client receiving each redundant feed.
   * @tparam R The time client.
   * @tparam T The timer driving feed expiry and gap deadlines.
   */
  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class OtcLinkClient {
    public:

      /** The protocol client receiving a feed. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The source of the current time. */
      using TimeClient = Beam::dereference_t<R>;

      /** The timer driving feed expiry and gap deadlines. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs an OtcLinkClient.
       * @param feed_timeout How long a silent or stalled feed delays a gap.
       * @param gap_timeout How long to wait before logging and skipping a gap.
       * @param feed_clients The redundant live feeds for one channel.
       * @param time_client The source of the current time.
       * @param timer The timer driving feed expiry and gap deadlines.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<R> RF,
        Beam::Initializes<T> TF>
      OtcLinkClient(boost::posix_time::time_duration feed_timeout,
        boost::posix_time::time_duration gap_timeout,
        std::vector<PF> feed_clients, RF&& time_client, TF&& timer);

      ~OtcLinkClient();

      /** Returns a message valid until the next read or client destruction. */
      OtcLinkMessage read();

      /**
       * Reads a message and its channel session number.
       * @param session Receives a counter incremented on channel resets.
       * @return A message valid until the next read or client destruction.
       */
      OtcLinkMessage read(Beam::Out<std::uint64_t> session);

      /** Closes the feeds and interrupts pending reads. */
      void close();

    private:
      struct Feed {
        std::uint64_t m_session = 0;
        boost::optional<boost::posix_time::ptime> m_reset;
        bool m_is_closed = false;
      };
      struct State {
        OtcLinkSequencer m_sequencer;
        std::vector<Feed> m_feeds;
        std::uint64_t m_session = 0;
        boost::optional<std::uint64_t> m_gap_sequence;
        boost::posix_time::ptime m_gap_timestamp;
        bool m_is_finished = false;
      };
      struct Message {
        Beam::SharedBuffer m_payload;
        std::uint64_t m_session;
      };
      boost::posix_time::time_duration m_gap_timeout;
      std::vector<Beam::local_ptr_t<P>> m_feed_clients;
      Beam::local_ptr_t<R> m_time_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Sync<State> m_state;
      Beam::Queue<Message> m_messages;
      Beam::SharedBuffer m_payload;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandlerGroup m_routines;
      Beam::OpenState m_open_state;

      static boost::posix_time::ptime get_timestamp(
        const OtcLinkHeader& header, boost::posix_time::ptime received);
      OtcLinkClient(const OtcLinkClient&) = delete;
      OtcLinkClient& operator =(const OtcLinkClient&) = delete;
      void reset(State& state, int feed, const OtcLinkHeader& header,
        boost::posix_time::ptime timestamp);
      void flush(State& state, boost::posix_time::ptime timestamp);
      void fail(const std::exception_ptr& error);
      void feed_loop(int feed);
      void on_timer(typename Timer::Result result);
  };

  template<typename P, typename R, typename T>
  OtcLinkClient(boost::posix_time::time_duration,
    boost::posix_time::time_duration, std::vector<P>, R&&, T&&) ->
      OtcLinkClient<P, std::remove_cvref_t<R>, std::remove_cvref_t<T>>;

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<P> PF, Beam::Initializes<R> RF,
    Beam::Initializes<T> TF>
  OtcLinkClient<P, R, T>::OtcLinkClient(
      boost::posix_time::time_duration feed_timeout,
      boost::posix_time::time_duration gap_timeout,
      std::vector<PF> feed_clients, RF&& time_client, TF&& timer)
      : m_gap_timeout(gap_timeout),
        m_feed_clients(std::make_move_iterator(feed_clients.begin()),
          std::make_move_iterator(feed_clients.end())),
        m_time_client(std::forward<RF>(time_client)),
        m_timer(std::forward<TF>(timer)),
        m_state(OtcLinkSequencer(
            static_cast<int>(m_feed_clients.size()), feed_timeout),
          std::vector<Feed>(m_feed_clients.size())) {
    try {
      if(gap_timeout.is_special() ||
          gap_timeout <= boost::posix_time::seconds(0)) {
        boost::throw_with_location(std::invalid_argument(
          "A positive OTC Link gap timeout is required."));
      }
      for(auto i = 0; i < static_cast<int>(m_feed_clients.size()); ++i) {
        m_routines.spawn(std::bind_front(&OtcLinkClient::feed_loop, this, i));
      }
      m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
        std::bind_front(&OtcLinkClient::on_timer, this)));
      m_timer->start();
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkClient<P, R, T>::~OtcLinkClient() {
    close();
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkMessage OtcLinkClient<P, R, T>::read() {
    auto session = std::uint64_t(0);
    return read(Beam::out(session));
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkMessage OtcLinkClient<P, R, T>::read(
      Beam::Out<std::uint64_t> session) {
    auto message = m_messages.pop();
    m_payload = std::move(message.m_payload);
    *session = message.m_session;
    return OtcLinkMessage::parse(
      std::string_view(m_payload.get_data(), m_payload.get_size()));
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkClient<P, R, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    fail(std::make_exception_ptr(Beam::EndOfFileException()));
    for(auto& client : m_feed_clients) {
      client->close();
    }
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_routines.wait();
    m_open_state.close();
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  boost::posix_time::ptime OtcLinkClient<P, R, T>::get_timestamp(
      const OtcLinkHeader& header, boost::posix_time::ptime received) {
    if(header.m_milliseconds >=
        boost::posix_time::hours(24).total_milliseconds()) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link packet timestamp."));
    }
    static auto time_zone =
      TIME_ZONES.time_zone_from_region("America/New_York");
    auto local_timestamp =
      boost::local_time::local_date_time(received, time_zone).local_time();
    auto timestamp = boost::posix_time::ptime(local_timestamp.date(),
      boost::posix_time::milliseconds(header.m_milliseconds));
    auto half_day = boost::posix_time::hours(12);
    if(timestamp - local_timestamp > half_day) {
      timestamp -= boost::gregorian::days(1);
    } else if(local_timestamp - timestamp > half_day) {
      timestamp += boost::gregorian::days(1);
    }
    return timestamp - (local_timestamp - received);
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkClient<P, R, T>::reset(State& state, int feed,
      const OtcLinkHeader& header, boost::posix_time::ptime timestamp) {
    auto reset = get_timestamp(header, timestamp);
    auto& source = state.m_feeds[feed];
    if(source.m_reset && reset <= *source.m_reset) {
      return;
    }
    source.m_reset = reset;
    ++source.m_session;
    if(source.m_session <= state.m_session) {
      return;
    }
    if(auto gap = state.m_sequencer.get_gap()) {
      std::osyncstream(std::cout) << "(dropped " << timestamp << ' ' <<
        gap->m_sequence << ' ' << gap->m_count << " session_reset)" <<
        std::endl;
    }
    state.m_session = source.m_session;
    state.m_sequencer.reset(1);
    state.m_gap_sequence = boost::none;
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkClient<P, R, T>::flush(
      State& state, boost::posix_time::ptime timestamp) {
    state.m_sequencer.update(timestamp);
    while(true) {
      while(auto payload = state.m_sequencer.read()) {
        m_messages.push(Message(std::move(*payload), state.m_session));
      }
      auto gap = state.m_sequencer.get_gap();
      if(!gap) {
        state.m_gap_sequence = boost::none;
        return;
      }
      if(state.m_gap_sequence != gap->m_sequence) {
        state.m_gap_sequence = gap->m_sequence;
        state.m_gap_timestamp = timestamp;
      }
      if(timestamp >= state.m_gap_timestamp &&
          timestamp - state.m_gap_timestamp <= m_gap_timeout) {
        return;
      }
      std::osyncstream(std::cout) << "(dropped " << timestamp << ' ' <<
        gap->m_sequence << ' ' << gap->m_count << " recovery_disabled)" <<
        std::endl;
      state.m_sequencer.skip(gap->m_count);
      state.m_gap_sequence = boost::none;
    }
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkClient<P, R, T>::fail(const std::exception_ptr& error) {
    Beam::with(m_state, [&] (auto& state) {
      state.m_is_finished = true;
      m_messages.close(error);
    });
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkClient<P, R, T>::feed_loop(int feed) {
    while(m_open_state.is_open()) {
      try {
        auto received = boost::posix_time::ptime();
        auto packet = m_feed_clients[feed]->read(Beam::out(received));
        auto& header = packet.get_header();
        auto is_replayed_reset =
          header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET) &&
          header.has_flag(OtcLinkHeader::Flag::REPLAY);
        if(header.has_flag(OtcLinkHeader::Flag::TEST) || is_replayed_reset) {
          continue;
        }
        auto timestamp = m_time_client->get_time();
        auto is_finished = Beam::with(m_state, [&] (auto& state) {
          if(state.m_is_finished) {
            return true;
          }
          if(header.has_flag(OtcLinkHeader::Flag::SEQUENCE_RESET)) {
            reset(state, feed, header, received);
          } else if(state.m_feeds[feed].m_session == state.m_session) {
            auto& source = state.m_feeds[feed];
            if(!source.m_reset ||
                get_timestamp(header, received) >= *source.m_reset) {
              state.m_sequencer.add(feed, packet, received);
            }
          }
          flush(state, timestamp);
          return false;
        });
        if(is_finished) {
          return;
        }
      } catch(const OtcLinkParserException& error) {
        std::osyncstream(std::cout) << "(bad_datagram " << feed << ' ' <<
          error.what() << ')' << std::endl;
      } catch(const std::exception& error) {
        auto exception = std::current_exception();
        Beam::with(m_state, [&] (auto& state) {
          state.m_feeds[feed].m_is_closed = true;
          if(state.m_is_finished) {
            return;
          }
          std::osyncstream(std::cout) << "(feed_failed " << feed << ' ' <<
            error.what() << ')' << std::endl;
          if(std::ranges::all_of(state.m_feeds, [] (const auto& feed) {
              return feed.m_is_closed;
            })) {
            state.m_is_finished = true;
            m_messages.close(exception);
          }
        });
        return;
      }
    }
  }

  template<typename P, typename R, typename T> requires
    IsOtcLinkProtocolClient<Beam::dereference_t<P>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkClient<P, R, T>::on_timer(typename Timer::Result result) {
    if(!m_open_state.is_open() || result == Timer::Result::CANCELED) {
      return;
    }
    try {
      if(result == Timer::Result::FAIL) {
        boost::throw_with_location(Beam::IOException("OTC Link timer failed."));
      }
      auto timestamp = m_time_client->get_time();
      auto is_finished = Beam::with(m_state, [&] (auto& state) {
        if(!state.m_is_finished) {
          flush(state, timestamp);
        }
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
