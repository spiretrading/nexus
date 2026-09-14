#ifndef CXA_PITCH_CLIENT_HPP
#define CXA_PITCH_CLIENT_HPP
#include <algorithm>
#include <exception>
#include <iterator>
#include <map>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchGapClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchReport.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSpinClient.hpp"

namespace Nexus {

  /** Concept satisfied by types delivering a unit's PITCH messages. */
  template<typename T>
  concept IsCxaPitchClient = requires(T& t) {
    { t.read() } -> std::same_as<CxaPitchMessage>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Delivers a unit's ordered CXA PITCH message stream.
   * @tparam P The type of client receiving a feed's blocks.
   * @tparam G The type of client requesting retransmissions.
   * @tparam S The type of client requesting a snapshot.
   * @tparam R The type of client used to get the current time.
   * @tparam T The type of timer used to detect a silent feed.
   */
  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  class CxaPitchClient {
    public:

      /** The type of client receiving a feed's blocks. */
      using ProtocolClient = Beam::dereference_t<P>;

      /** The type of client requesting retransmissions. */
      using GapClient = Beam::dereference_t<G>;

      /** The type of client requesting a snapshot. */
      using SpinClient = Beam::dereference_t<S>;

      /** The type of client used to get the current time. */
      using TimeClient = Beam::dereference_t<R>;

      /** The type of timer used to detect a silent feed. */
      using Timer = Beam::dereference_t<T>;

      /** The number of rejected snapshot requests to tolerate. */
      static constexpr auto SPIN_ATTEMPTS = 10;

      /**
       * Constructs a CxaPitchClient.
       * @param unit The unit to deliver the messages of.
       * @param feed_timeout How long a feed may be silent before it is
       *        excluded.
       * @param gap_timeout How long to wait for missing messages or an initial
       *        snapshot offer before proceeding without them.
       * @param feed_clients The clients receiving the unit's real time feeds.
       * @param recovery_clients The clients receiving the unit's gap response
       *        feeds.
       * @param gap_client The client requesting retransmissions, if enabled.
       * @param spin_client The client requesting the initial snapshot, if
       *        enabled.
       * @param time_client The client used to get the current time.
       * @param timer The timer measuring the period between silence checks.
       */
      template<Beam::Initializes<P> PF, Beam::Initializes<G> GF,
        Beam::Initializes<S> SF, Beam::Initializes<R> RF,
        Beam::Initializes<T> TF>
      CxaPitchClient(
        std::uint8_t unit, boost::posix_time::time_duration feed_timeout,
        boost::posix_time::time_duration gap_timeout,
        std::vector<PF> feed_clients, std::vector<PF> recovery_clients,
        boost::optional<GF> gap_client, boost::optional<SF> spin_client,
        RF&& time_client, TF&& timer);

      ~CxaPitchClient();

      /** Reads the next message in sequence. */
      CxaPitchMessage read();

      /** Closes the connection to every feed and server. */
      void close();

    private:
      struct Rejection {
        std::uint32_t m_end;
        std::string m_reason;
      };
      std::uint8_t m_unit;
      boost::posix_time::time_duration m_feed_timeout;
      boost::posix_time::time_duration m_gap_timeout;
      std::vector<Beam::local_ptr_t<P>> m_feed_clients;
      std::vector<Beam::local_ptr_t<P>> m_recovery_clients;
      boost::optional<Beam::local_ptr_t<G>> m_gap_client;
      boost::optional<Beam::local_ptr_t<S>> m_spin_client;
      Beam::local_ptr_t<R> m_time_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Sync<CxaPitchSequencer> m_sequencer;
      Beam::Queue<Beam::SharedBuffer> m_messages;
      Beam::StateQueue<std::uint32_t> m_requests;
      Beam::SharedBuffer m_payload;
      std::uint32_t m_reported_gap;
      std::uint32_t m_requested;
      std::map<std::uint32_t, Rejection> m_rejections;
      boost::posix_time::ptime m_start;
      boost::posix_time::ptime m_gap_timestamp;
      boost::posix_time::ptime m_feed_timestamp;
      bool m_is_ready;
      bool m_is_spinning;
      bool m_is_silent;
      Beam::RoutineTaskQueue m_tasks;
      Beam::RoutineHandlerGroup m_routines;
      Beam::OpenState m_open_state;

      CxaPitchClient(const CxaPitchClient&) = delete;
      CxaPitchClient& operator =(const CxaPitchClient&) = delete;
      void flush(CxaPitchSequencer& sequencer);
      void drop(CxaPitchSequencer& sequencer, const CxaPitchGap& gap,
        boost::posix_time::ptime timestamp, std::string_view reason);
      void skip(
        CxaPitchSequencer& sequencer, boost::posix_time::ptime timestamp);
      void reject(
        std::uint32_t sequence, std::uint32_t count, std::string reason);
      boost::optional<CxaPitchGap> update_gap(
        CxaPitchSequencer& sequencer, boost::posix_time::ptime timestamp);
      boost::optional<std::uint32_t> advance_recovery(
        CxaPitchSequencer& sequencer, boost::posix_time::ptime timestamp);
      void feed_loop(int index);
      void recovery_loop(int index);
      void request_loop();
      void response_loop();
      void spin_loop();
      void on_timer(typename Timer::Result result);
  };

  template<typename P, typename G, typename S, typename R, typename T>
  CxaPitchClient(std::uint8_t, boost::posix_time::time_duration,
    boost::posix_time::time_duration, std::vector<P>, std::vector<P>,
    boost::optional<G>, boost::optional<S>, R&&, T&&) ->
      CxaPitchClient<P, G, S, std::remove_cvref_t<R>, std::remove_cvref_t<T>>;

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<P> PF, Beam::Initializes<G> GF,
    Beam::Initializes<S> SF, Beam::Initializes<R> RF, Beam::Initializes<T> TF>
  CxaPitchClient<P, G, S, R, T>::CxaPitchClient(
      std::uint8_t unit, boost::posix_time::time_duration feed_timeout,
      boost::posix_time::time_duration gap_timeout,
      std::vector<PF> feed_clients, std::vector<PF> recovery_clients,
      boost::optional<GF> gap_client, boost::optional<SF> spin_client,
      RF&& time_client, TF&& timer)
      try : m_unit(unit),
            m_feed_timeout(feed_timeout),
            m_gap_timeout(gap_timeout),
            m_feed_clients(std::make_move_iterator(feed_clients.begin()),
              std::make_move_iterator(feed_clients.end())),
            m_recovery_clients(
              std::make_move_iterator(recovery_clients.begin()),
              std::make_move_iterator(recovery_clients.end())),
            m_gap_client(std::move(gap_client)),
            m_spin_client(std::move(spin_client)),
            m_time_client(std::forward<RF>(time_client)),
            m_timer(std::forward<TF>(timer)),
            m_sequencer(static_cast<int>(m_feed_clients.size()), feed_timeout),
            m_reported_gap(0),
            m_requested(0),
            m_start(m_time_client->get_time()),
            m_gap_timestamp(m_start),
            m_feed_timestamp(m_start),
            m_is_ready(!m_spin_client),
            m_is_spinning(false),
            m_is_silent(false) {
    for(auto i = std::size_t(0); i != m_feed_clients.size(); ++i) {
      m_routines.spawn(
        std::bind_front(&CxaPitchClient::feed_loop, this, static_cast<int>(i)));
    }
    for(auto i = std::size_t(0); i != m_recovery_clients.size(); ++i) {
      m_routines.spawn(std::bind_front(
        &CxaPitchClient::recovery_loop, this, static_cast<int>(i)));
    }
    if(m_gap_client) {
      m_routines.spawn(std::bind_front(&CxaPitchClient::request_loop, this));
      m_routines.spawn(std::bind_front(&CxaPitchClient::response_loop, this));
    }
    if(m_spin_client) {
      m_routines.spawn(std::bind_front(&CxaPitchClient::spin_loop, this));
    }
    m_timer->get_publisher().monitor(m_tasks.get_slot<typename Timer::Result>(
      std::bind_front(&CxaPitchClient::on_timer, this)));
    m_timer->start();
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(
      Beam::ConnectException("Failed to initialize the CXA PITCH client."));
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  CxaPitchClient<P, G, S, R, T>::~CxaPitchClient() {
    close();
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  CxaPitchMessage CxaPitchClient<P, G, S, R, T>::read() {
    m_payload = m_messages.pop();
    return CxaPitchMessage::parse(
      std::string_view(m_payload.get_data(), m_payload.get_size()));
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    if(m_spin_client) {
      (*m_spin_client)->close();
    }
    if(m_gap_client) {
      (*m_gap_client)->close();
    }
    for(auto& client : m_recovery_clients) {
      client->close();
    }
    for(auto& client : m_feed_clients) {
      client->close();
    }
    m_tasks.close();
    m_tasks.wait();
    m_timer->cancel();
    m_messages.close();
    m_requests.close();
    m_routines.wait();
    m_open_state.close();
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::flush(CxaPitchSequencer& sequencer) {
    if(!m_is_ready) {
      return;
    }
    while(auto payload = sequencer.read()) {
      m_messages.push(std::move(*payload));
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::drop(CxaPitchSequencer& sequencer,
      const CxaPitchGap& gap, boost::posix_time::ptime timestamp,
      std::string_view reason) {
    print([&] (auto& out) {
      out << "(dropped " << timestamp << ' ' << gap.m_sequence << ' ' <<
        gap.m_count << ' ' << reason << ')';
    });
    sequencer.reset(gap.m_sequence + gap.m_count);
    m_reported_gap = 0;
    flush(sequencer);
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::skip(
      CxaPitchSequencer& sequencer, boost::posix_time::ptime timestamp) {
    while(!m_rejections.empty()) {
      auto gap = sequencer.get_gap();
      if(!gap) {
        break;
      }
      auto rejection = m_rejections.upper_bound(gap->m_sequence);
      if(rejection == m_rejections.begin()) {
        break;
      }
      --rejection;
      if(rejection->second.m_end <= gap->m_sequence) {
        break;
      }
      auto end =
        std::min(rejection->second.m_end, gap->m_sequence + gap->m_count);
      drop(sequencer, CxaPitchGap(gap->m_sequence, end - gap->m_sequence),
        timestamp, rejection->second.m_reason);
    }
    auto sequence = sequencer.get_sequence().value_or(0);
    while(!m_rejections.empty() &&
        m_rejections.begin()->second.m_end <= sequence) {
      m_rejections.erase(m_rejections.begin());
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::reject(
      std::uint32_t sequence, std::uint32_t count, std::string reason) {
    auto end = sequence + count;
    auto rejection = m_rejections.find(sequence);
    if(rejection == m_rejections.end()) {
      m_rejections.emplace(sequence, Rejection(end, std::move(reason)));
    } else if(rejection->second.m_end < end) {
      rejection->second.m_end = end;
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  boost::optional<CxaPitchGap> CxaPitchClient<P, G, S, R, T>::update_gap(
      CxaPitchSequencer& sequencer, boost::posix_time::ptime timestamp) {
    flush(sequencer);
    skip(sequencer, timestamp);
    auto gap = sequencer.get_gap();
    if(gap && gap->m_sequence != m_reported_gap) {
      m_reported_gap = gap->m_sequence;
      m_gap_timestamp = timestamp;
    }
    return gap;
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  boost::optional<std::uint32_t>
      CxaPitchClient<P, G, S, R, T>::advance_recovery(
        CxaPitchSequencer& sequencer, boost::posix_time::ptime timestamp) {
    if(auto gap = update_gap(sequencer, timestamp)) {
      if(m_gap_client &&
          !(*m_gap_client)->is_recoverable(*gap, sequencer.get_position())) {
        drop(sequencer, *gap, timestamp, "unrecoverable");
        return boost::none;
      }
      if(timestamp - m_gap_timestamp > m_gap_timeout) {
        if(m_gap_client) {
          drop(sequencer, *gap, timestamp, "timeout");
        } else {
          drop(sequencer, *gap, timestamp, "disabled");
        }
        return boost::none;
      }
      if(m_gap_client && m_requested < gap->m_sequence + gap->m_count) {
        return sequencer.get_position();
      }
    }
    return boost::none;
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::feed_loop(int index) {
    while(true) {
      try {
        auto block = m_feed_clients[index]->read();
        auto& header = block.get_header();
        if(header.m_unit != m_unit) {
          m_messages.close(std::make_exception_ptr(Beam::IOException(
            "CXA PITCH feed " + std::to_string(index) + " received unit " +
            std::to_string(header.m_unit) + "; expected unit " +
            std::to_string(m_unit) + ".")));
          return;
        }
        validate(block);
        auto timestamp = m_time_client->get_time();
        auto [close_spin, recovery_position] = Beam::with(m_sequencer,
          [&] (auto& sequencer) {
            sequencer.add(index, block, timestamp);
            m_feed_timestamp = timestamp;
            if(m_is_silent) {
              m_is_silent = false;
              print([&] (auto& out) {
                out << "(feed " << timestamp << ')';
              });
            }
            auto close_spin = [&] {
              if(m_is_ready || m_is_spinning ||
                  timestamp - m_start <= m_gap_timeout) {
                return false;
              }
              print([&] (auto& out) {
                out << "(no_spin " << timestamp << ')';
              });
              m_is_ready = true;
              return true;
            }();
            return std::pair(
              close_spin, advance_recovery(sequencer, timestamp));
          });
        if(close_spin) {
          (*m_spin_client)->close();
        }
        if(recovery_position) {
          m_requests.push(*recovery_position);
        }
      } catch(const CxaPitchParserException& e) {
        print([&] (auto& out) {
          out << "(bad_datagram feed " << index << ' ' << e.what() << ')';
        });
        continue;
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::recovery_loop(int index) {
    while(true) {
      try {
        auto block = m_recovery_clients[index]->read();
        auto& header = block.get_header();
        if(header.m_unit != m_unit) {
          m_messages.close(std::make_exception_ptr(Beam::IOException(
            "CXA PITCH recovery feed " + std::to_string(index) +
            " received unit " + std::to_string(header.m_unit) +
            "; expected unit " + std::to_string(m_unit) + ".")));
          return;
        }
        auto timestamp = m_time_client->get_time();
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          try {
            validate(block);
            sequencer.recover(block);
          } catch(const CxaPitchParserException&) {
            if(header.m_sequence != 0 && header.m_count != 0) {
              reject(header.m_sequence, header.m_count, "malformed");
            }
          }
          flush(sequencer);
          skip(sequencer, timestamp);
          if(!sequencer.get_gap()) {
            m_reported_gap = 0;
          }
        });
      } catch(const CxaPitchParserException& e) {
        print([&] (auto& out) {
          out << "(bad_datagram recovery " << index << ' ' << e.what() << ')';
        });
        continue;
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::request_loop() {
    while(true) {
      try {
        auto recovery_position = m_requests.pop();
        auto requested_gap = Beam::with(m_sequencer,
          [&] (const auto& sequencer) -> boost::optional<CxaPitchGap> {
            if(auto gap = sequencer.get_gap()) {
              if(m_requested < gap->m_sequence) {
                m_requested = gap->m_sequence;
              }
              auto end = gap->m_sequence + gap->m_count;
              if(m_requested < end) {
                return CxaPitchGap(m_requested, end - m_requested);
              }
            }
            return boost::none;
          });
        if(!requested_gap) {
          continue;
        }
        auto count = [&] {
          try {
            return (*m_gap_client)->request(
              m_unit, *requested_gap, recovery_position);
          } catch(const std::exception&) {
            return std::uint32_t(0);
          }
        }();
        Beam::with(m_sequencer, [&] (const auto&) {
          if(m_requested == requested_gap->m_sequence) {
            m_requested = requested_gap->m_sequence + count;
          }
        });
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::response_loop() {
    while(true) {
      try {
        auto response = (*m_gap_client)->get_responses()->pop();
        if(response.m_status == CxaPitchGapResponse::ACCEPTED) {
          continue;
        }
        auto timestamp = m_time_client->get_time();
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          if(response.m_status == CxaPitchGapResponse::MINUTE_EXHAUSTED ||
              response.m_status == CxaPitchGapResponse::SECOND_EXHAUSTED) {
            if(m_requested > response.m_sequence) {
              m_requested = response.m_sequence;
            }
            return;
          }
          reject(response.m_sequence, response.m_count,
            "rejected " + std::string(1, response.m_status));
          skip(sequencer, timestamp);
        });
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::spin_loop() {
    auto attempts = 0;
    try {
      auto sequences = std::make_shared<Beam::StateQueue<std::uint32_t>>();
      (*m_spin_client)->monitor_snapshot_sequences(sequences);
      while(attempts != SPIN_ATTEMPTS) {
        auto offer = sequences->pop();
        auto is_requested = Beam::with(m_sequencer,
          [&] (const auto& sequencer) {
            auto sequence = sequencer.get_sequence();
            if(m_is_ready || !sequence || offer + 1 < *sequence) {
              return false;
            }
            m_is_spinning = true;
            return true;
          });
        if(!is_requested) {
          continue;
        }
        auto snapshot = (*m_spin_client)->load_snapshot(offer);
        if(snapshot.m_status != CxaPitchSpinResponse::ACCEPTED) {
          Beam::with(m_sequencer, [&] (const auto&) {
            m_is_spinning = false;
          });
          ++attempts;
          continue;
        }
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          if(m_is_ready) {
            return;
          }
          for(auto& message : snapshot.m_messages) {
            m_messages.push(message);
          }
          if(sequencer.get_sequence().value_or(0) < snapshot.m_sequence + 1) {
            sequencer.reset(snapshot.m_sequence + 1);
          }
          m_is_ready = true;
          flush(sequencer);
        });
        break;
      }
    } catch(const std::exception&) {}
    try {
      Beam::with(m_sequencer, [&] (auto& sequencer) {
        if(!m_is_ready) {
          if(m_open_state.is_open()) {
            auto timestamp = m_time_client->get_time();
            print([&] (auto& out) {
              out << "(no_spin " << timestamp << ')';
            });
          }
          m_is_ready = true;
          flush(sequencer);
        }
      });
    } catch(const std::exception&) {}
    (*m_spin_client)->close();
  }

  template<typename P, typename G, typename S, typename R, typename T> requires
    IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::on_timer(typename Timer::Result result) {
    if(result != Timer::Result::EXPIRED) {
      return;
    }
    try {
      auto timestamp = m_time_client->get_time();
      Beam::with(m_sequencer, [&] (auto& sequencer) {
        if(!m_is_silent && timestamp - m_feed_timestamp > m_feed_timeout) {
          m_is_silent = true;
          print([&] (auto& out) {
            out << "(no_feed " << timestamp << ')';
          });
        }
        auto previous_gap = m_reported_gap;
        if(auto gap = update_gap(sequencer, timestamp)) {
          if(gap->m_sequence == previous_gap &&
              timestamp - m_gap_timestamp > m_gap_timeout) {
            drop(sequencer, *gap, timestamp, "timeout");
          }
        }
      });
      m_timer->start();
    } catch(const std::exception&) {
      m_tasks.close();
    }
  }
}

#endif
