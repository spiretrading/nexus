#ifndef CXA_PITCH_CLIENT_HPP
#define CXA_PITCH_CLIENT_HPP
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/TimeService/Timer.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchGapClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchReport.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSequencer.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSpinClient.hpp"

namespace Nexus {

  /** Concept satisfied by types delivering a unit's PITCH messages. */
  template<typename T>
  concept IsCxaPitchClient = requires(T& t) {
    { t.read() } -> std::same_as<CxaPitchMessage>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Delivers a unit's CXA PITCH messages in sequence, arbitrating its feeds,
   * recovering the messages missing from all of them and building the initial
   * state of its book.
   * @param <P> The type of client receiving a feed's blocks.
   * @param <G> The type of client requesting retransmissions.
   * @param <S> The type of client requesting a snapshot.
   * @param <R> The type of client used to get the current time.
   * @param <T> The type of Timer used to detect a silent feed.
   */
  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
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
       * @param liveness How long a feed may be silent before it is excluded.
       * @param gap_timeout How long to wait for missing messages or snapshot
       *        progress before proceeding without them.
       * @param feeds The clients receiving the unit's real time feeds.
       * @param recovery The clients receiving the unit's gap response feeds.
       * @param gap The client requesting retransmissions, if enabled.
       * @param spin The client requesting the initial snapshot, if enabled.
       * @param time_client The client used to get the current time.
       * @param timer The timer measuring the period between silence checks.
       */
      template<Beam::Initializes<R> RF, Beam::Initializes<T> TF>
      CxaPitchClient(std::uint8_t unit,
        boost::posix_time::time_duration liveness,
        boost::posix_time::time_duration gap_timeout, std::vector<P> feeds,
        std::vector<P> recovery, boost::optional<G> gap,
        boost::optional<S> spin, RF&& time_client, TF&& timer);

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
      boost::posix_time::time_duration m_liveness;
      boost::posix_time::time_duration m_gap_timeout;
      std::vector<Beam::local_ptr_t<P>> m_feeds;
      std::vector<Beam::local_ptr_t<P>> m_recovery;
      boost::optional<Beam::local_ptr_t<G>> m_gap;
      boost::optional<Beam::local_ptr_t<S>> m_spin;
      Beam::local_ptr_t<R> m_time_client;
      Beam::local_ptr_t<T> m_timer;
      Beam::Sync<CxaPitchSequencer> m_sequencer;
      Beam::Queue<Beam::SharedBuffer> m_messages;
      Beam::StateQueue<std::uint32_t> m_requests;
      Beam::SharedBuffer m_payload;
      std::uint32_t m_reported_gap;
      std::uint32_t m_requested;
      std::map<std::uint32_t, Rejection> m_rejections;
      std::uint64_t m_spin_progress;
      boost::posix_time::ptime m_start;
      boost::posix_time::ptime m_spin_timestamp;
      boost::posix_time::ptime m_gap_timestamp;
      boost::posix_time::ptime m_feed_timestamp;
      bool m_is_ready;
      bool m_is_spinning;
      bool m_is_silent;
      std::shared_ptr<Beam::Queue<Beam::Timer::Result>> m_timer_queue;
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
      void feed_loop(int index);
      void recovery_loop(int index);
      void request_loop();
      void response_loop();
      void spin_loop();
      void watch_loop();
  };

  template<typename P, typename G, typename S, typename R, typename T>
  CxaPitchClient(std::uint8_t, boost::posix_time::time_duration,
    boost::posix_time::time_duration, std::vector<P>, std::vector<P>,
    boost::optional<G>, boost::optional<S>, R&&, T&&) ->
      CxaPitchClient<P, G, S, std::remove_cvref_t<R>, std::remove_cvref_t<T>>;

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<R> RF, Beam::Initializes<T> TF>
  CxaPitchClient<P, G, S, R, T>::CxaPitchClient(std::uint8_t unit,
      boost::posix_time::time_duration liveness,
      boost::posix_time::time_duration gap_timeout, std::vector<P> feeds,
      std::vector<P> recovery, boost::optional<G> gap, boost::optional<S> spin,
      RF&& time_client, TF&& timer)
      try : m_unit(unit),
            m_liveness(liveness),
            m_gap_timeout(gap_timeout),
            m_feeds(std::make_move_iterator(feeds.begin()),
              std::make_move_iterator(feeds.end())),
            m_recovery(std::make_move_iterator(recovery.begin()),
              std::make_move_iterator(recovery.end())),
            m_gap(std::move(gap)),
            m_spin(std::move(spin)),
            m_time_client(std::forward<RF>(time_client)),
            m_timer(std::forward<TF>(timer)),
            m_sequencer(static_cast<int>(m_feeds.size()), liveness),
            m_reported_gap(0),
            m_requested(0),
            m_spin_progress(0),
            m_start(m_time_client->get_time()),
            m_spin_timestamp(m_start),
            m_gap_timestamp(m_start),
            m_feed_timestamp(m_start),
            m_is_ready(!m_spin),
            m_is_spinning(false),
            m_is_silent(false),
            m_timer_queue(
              std::make_shared<Beam::Queue<Beam::Timer::Result>>()) {
    for(auto i = std::size_t(0); i != m_feeds.size(); ++i) {
      m_routines.spawn(
        std::bind_front(&CxaPitchClient::feed_loop, this, static_cast<int>(i)));
    }
    for(auto i = std::size_t(0); i != m_recovery.size(); ++i) {
      m_routines.spawn(std::bind_front(
        &CxaPitchClient::recovery_loop, this, static_cast<int>(i)));
    }
    if(m_gap) {
      m_routines.spawn(std::bind_front(&CxaPitchClient::request_loop, this));
      m_routines.spawn(std::bind_front(&CxaPitchClient::response_loop, this));
    }
    if(m_spin) {
      m_routines.spawn(std::bind_front(&CxaPitchClient::spin_loop, this));
    }
    m_timer->get_publisher().monitor(m_timer_queue);
    m_timer->start();
    m_routines.spawn(std::bind_front(&CxaPitchClient::watch_loop, this));
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(
      Beam::ConnectException("Failed to initialize the CXA PITCH client."));
  }

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  CxaPitchClient<P, G, S, R, T>::~CxaPitchClient() {
    close();
  }

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  CxaPitchMessage CxaPitchClient<P, G, S, R, T>::read() {
    m_payload = m_messages.pop();
    return CxaPitchMessage::parse(
      std::string_view(m_payload.get_data(), m_payload.get_size()));
  }

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    if(m_spin) {
      (*m_spin)->close();
    }
    if(m_gap) {
      (*m_gap)->close();
    }
    for(auto& client : m_recovery) {
      client->close();
    }
    for(auto& client : m_feeds) {
      client->close();
    }
    m_timer->cancel();
    m_timer_queue->close();
    m_messages.close();
    m_requests.close();
    m_routines.wait();
    m_open_state.close();
  }

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
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

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
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

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
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

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
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

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::feed_loop(int index) {
    while(true) {
      try {
        auto block = m_feeds[index]->read();
        auto& header = block.get_header();
        if(header.m_unit != m_unit) {
          continue;
        }
        auto timestamp = m_time_client->get_time();
        auto live = boost::optional<std::uint32_t>();
        auto close_spin = false;
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          auto is_restart = sequencer.add(index, block, timestamp);
          m_feed_timestamp = timestamp;
          if(m_is_silent) {
            m_is_silent = false;
            print([&] (auto& out) {
              out << "(feed " << timestamp << ')';
            });
          }
          if(is_restart) {
            print([&] (auto& out) {
              out << "(restarted " << timestamp << ' ' <<
                block.get_header().m_sequence << ')';
            });
            m_reported_gap = 0;
            m_requested = 0;
            m_rejections.clear();
          }
          if(!m_is_ready) {
            auto start = m_start;
            if(m_is_spinning) {
              auto progress = (*m_spin)->get_progress();
              if(progress != m_spin_progress) {
                m_spin_progress = progress;
                m_spin_timestamp = timestamp;
              }
              start = m_spin_timestamp;
            }
            if(timestamp - start > m_gap_timeout) {
              print([&] (auto& out) {
                out << "(no_spin " << timestamp << ')';
              });
              m_is_ready = true;
              close_spin = true;
            }
          }
          flush(sequencer);
          skip(sequencer, timestamp);
          auto gap = sequencer.get_gap();
          if(!gap) {
            return;
          }
          if(gap->m_sequence != m_reported_gap) {
            m_reported_gap = gap->m_sequence;
            m_gap_timestamp = timestamp;
          }
          auto reason = [&] () -> std::string_view {
            if(m_gap &&
                !(*m_gap)->is_recoverable(*gap, sequencer.get_position())) {
              return "unrecoverable";
            }
            if(timestamp - m_gap_timestamp <= m_gap_timeout) {
              return {};
            }
            if(m_gap) {
              return "timeout";
            }
            return "disabled";
          }();
          if(!reason.empty()) {
            drop(sequencer, *gap, timestamp, reason);
            return;
          }
          if(m_gap && m_requested < gap->m_sequence + gap->m_count) {
            live = sequencer.get_position();
          }
        });
        if(close_spin) {
          (*m_spin)->close();
        }
        if(live) {
          m_requests.push(*live);
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

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::recovery_loop(int index) {
    while(true) {
      try {
        auto block = m_recovery[index]->read();
        if(block.get_header().m_unit != m_unit) {
          continue;
        }
        auto timestamp = m_time_client->get_time();
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          try {
            try {
            sequencer.recover(block);
          } catch(const CxaPitchParserException&) {
            auto& header = block.get_header();
            if(header.m_sequence != 0 && header.m_count != 0) {
              reject(header.m_sequence, header.m_count, "malformed");
            }
          }
          } catch(const CxaPitchParserException&) {
            auto& header = block.get_header();
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

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::request_loop() {
    while(true) {
      try {
        auto live = m_requests.pop();
        auto pending = Beam::with(m_sequencer, [&] (auto& sequencer) {
          auto request = boost::optional<CxaPitchGap>();
          auto gap = sequencer.get_gap();
          if(gap) {
            if(m_requested < gap->m_sequence) {
              m_requested = gap->m_sequence;
            }
            auto end = gap->m_sequence + gap->m_count;
            if(m_requested < end) {
              request = CxaPitchGap(m_requested, end - m_requested);
            }
          }
          return request;
        });
        if(!pending) {
          continue;
        }
        auto count = [&] {
          try {
            return (*m_gap)->request(m_unit, *pending, live);
          } catch(const std::exception&) {
            return std::uint32_t(0);
          }
        }();
        Beam::with(m_sequencer, [&] (auto&) {
          if(m_requested == pending->m_sequence) {
            m_requested = pending->m_sequence + count;
          }
        });
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::response_loop() {
    while(true) {
      try {
        auto response = (*m_gap)->get_responses()->pop();
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

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::spin_loop() {
    auto attempts = 0;
    try {
      while(attempts != SPIN_ATTEMPTS) {
        auto offer = (*m_spin)->get_offers()->pop();
        auto is_requested = Beam::with(m_sequencer, [&] (auto& sequencer) {
          auto sequence = sequencer.get_sequence();
          if(m_is_ready || !sequence || offer + 1 < *sequence) {
            return false;
          }
          m_is_spinning = true;
          m_spin_progress = (*m_spin)->get_progress();
          m_spin_timestamp = m_time_client->get_time();
          return true;
        });
        if(!is_requested) {
          continue;
        }
        auto spin = (*m_spin)->request(offer);
        if(spin.m_status != CxaPitchSpinResponse::ACCEPTED) {
          Beam::with(m_sequencer, [&] (auto& sequencer) {
            m_is_spinning = false;
          });
          ++attempts;
          continue;
        }
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          if(m_is_ready) {
            return;
          }
          for(auto& message : spin.m_messages) {
            m_messages.push(message);
          }
          if(sequencer.get_sequence().value_or(0) < spin.m_sequence + 1) {
            if(sequencer.get_sequence().value_or(0) < spin.m_sequence + 1) {
            sequencer.reset(spin.m_sequence + 1);
          }
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
    (*m_spin)->close();
  }

  template<typename P, typename G, typename S, typename R, typename T>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>> &&
      Beam::IsTimer<Beam::dereference_t<T>>
  void CxaPitchClient<P, G, S, R, T>::watch_loop() {
    while(true) {
      try {
        if(m_timer_queue->pop() != Beam::Timer::Result::EXPIRED) {
          break;
        }
        auto timestamp = m_time_client->get_time();
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          if(!m_is_silent && timestamp - m_feed_timestamp > m_liveness) {
            m_is_silent = true;
            print([&] (auto& out) {
              out << "(no_feed " << timestamp << ')';
            });
          }
          flush(sequencer);
          skip(sequencer, timestamp);
          auto gap = sequencer.get_gap();
          if(!gap) {
            return;
          }
          if(gap->m_sequence != m_reported_gap) {
            m_reported_gap = gap->m_sequence;
            m_gap_timestamp = timestamp;
          } else if(timestamp - m_gap_timestamp > m_gap_timeout) {
            drop(sequencer, *gap, timestamp, "timeout");
          }
        });
        m_timer->start();
      } catch(const std::exception&) {
        break;
      }
    }
  }
}

#endif
