#ifndef CXA_PITCH_CLIENT_HPP
#define CXA_PITCH_CLIENT_HPP
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchGapClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"
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
   */
  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
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

      /** The number of rejected snapshot requests to tolerate. */
      static constexpr auto SPIN_ATTEMPTS = 10;

      /**
       * Constructs a CxaPitchClient.
       * @param unit The unit to deliver the messages of.
       * @param liveness How long a feed may be silent before it is excluded.
       * @param gap_timeout How long to wait for a missing message before
       *        skipping over it.
       * @param feeds The clients receiving the unit's real time feeds.
       * @param recovery The clients receiving the unit's gap response feeds.
       * @param gap The client requesting retransmissions, if enabled.
       * @param spin The client requesting the initial snapshot, if enabled.
       * @param time_client The client used to get the current time.
       */
      template<Beam::Initializes<R> RF>
      CxaPitchClient(std::uint8_t unit,
        boost::posix_time::time_duration liveness,
        boost::posix_time::time_duration gap_timeout, std::vector<P> feeds,
        std::vector<P> recovery, boost::optional<G> gap,
        boost::optional<S> spin, RF&& time_client);

      ~CxaPitchClient();

      /** Reads the next message in sequence. */
      CxaPitchMessage read();

      /** Closes the connection to every feed and server. */
      void close();

    private:
      std::uint8_t m_unit;
      boost::posix_time::time_duration m_gap_timeout;
      std::vector<P> m_feeds;
      std::vector<P> m_recovery;
      boost::optional<G> m_gap;
      boost::optional<S> m_spin;
      Beam::local_ptr_t<R> m_time_client;
      Beam::Sync<CxaPitchSequencer> m_sequencer;
      std::shared_ptr<Beam::Queue<Beam::SharedBuffer>> m_messages;
      Beam::SharedBuffer m_payload;
      std::uint32_t m_reported_gap;
      std::uint32_t m_requested;
      std::uint32_t m_live;
      boost::posix_time::ptime m_gap_timestamp;
      bool m_is_ready;
      Beam::RoutineHandlerGroup m_routines;
      Beam::OpenState m_open_state;

      CxaPitchClient(const CxaPitchClient&) = delete;
      CxaPitchClient& operator =(const CxaPitchClient&) = delete;
      void flush(CxaPitchSequencer& sequencer);
      void feed_loop(int index);
      void recovery_loop(int index);
      void response_loop();
      void spin_loop();
  };

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  template<Beam::Initializes<R> RF>
  CxaPitchClient<P, G, S, R>::CxaPitchClient(std::uint8_t unit,
      boost::posix_time::time_duration liveness,
      boost::posix_time::time_duration gap_timeout, std::vector<P> feeds,
      std::vector<P> recovery, boost::optional<G> gap, boost::optional<S> spin,
      RF&& time_client)
      try : m_unit(unit),
            m_gap_timeout(gap_timeout),
            m_feeds(std::move(feeds)),
            m_recovery(std::move(recovery)),
            m_gap(std::move(gap)),
            m_spin(std::move(spin)),
            m_time_client(std::forward<RF>(time_client)),
            m_sequencer(static_cast<int>(m_feeds.size()), liveness),
            m_messages(std::make_shared<Beam::Queue<Beam::SharedBuffer>>()),
            m_reported_gap(0),
            m_requested(0),
            m_live(0),
            m_gap_timestamp(m_time_client->get_time()),
            m_is_ready(!m_spin) {
    for(auto i = std::size_t(0); i != m_feeds.size(); ++i) {
      m_routines.spawn(
        std::bind_front(&CxaPitchClient::feed_loop, this, static_cast<int>(i)));
    }
    for(auto i = std::size_t(0); i != m_recovery.size(); ++i) {
      m_routines.spawn(std::bind_front(
        &CxaPitchClient::recovery_loop, this, static_cast<int>(i)));
    }
    if(m_gap) {
      m_routines.spawn(std::bind_front(&CxaPitchClient::response_loop, this));
    }
    if(m_spin) {
      m_routines.spawn(std::bind_front(&CxaPitchClient::spin_loop, this));
    }
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(
      Beam::ConnectException("Failed to initialize the CXA PITCH client."));
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  CxaPitchClient<P, G, S, R>::~CxaPitchClient() {
    close();
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  CxaPitchMessage CxaPitchClient<P, G, S, R>::read() {
    m_payload = m_messages->pop();
    return CxaPitchMessage::parse(
      std::string_view(m_payload.get_data(), m_payload.get_size()));
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void CxaPitchClient<P, G, S, R>::close() {
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
    m_messages->close();
    m_routines.wait();
    m_open_state.close();
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void CxaPitchClient<P, G, S, R>::flush(CxaPitchSequencer& sequencer) {
    if(!m_is_ready) {
      return;
    }
    while(auto payload = sequencer.read()) {
      m_messages->push(std::move(*payload));
    }
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void CxaPitchClient<P, G, S, R>::feed_loop(int index) {
    while(true) {
      try {
        auto block = m_feeds[index]->read();
        auto& header = block.get_header();
        if(header.m_unit != m_unit) {
          continue;
        }
        auto timestamp = m_time_client->get_time();
        auto pending = boost::optional<CxaPitchGap>();
        auto position = std::uint32_t(0);
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          sequencer.add(index, block, timestamp);
          flush(sequencer);
          if(header.m_sequence + header.m_count > m_live) {
            m_live = header.m_sequence + header.m_count;
          }
          auto gap = sequencer.get_gap();
          if(!gap) {
            return;
          }
          if(gap->m_sequence != m_reported_gap) {
            m_reported_gap = gap->m_sequence;
            m_requested = gap->m_sequence;
            m_gap_timestamp = timestamp;
          } else if(timestamp - m_gap_timestamp > m_gap_timeout) {
            std::cout << "(dropped " << timestamp << ' ' << gap->m_sequence <<
              ' ' << gap->m_count << ')' << std::endl;
            sequencer.reset(gap->m_sequence + gap->m_count);
            m_reported_gap = 0;
            flush(sequencer);
            return;
          }
          auto end = gap->m_sequence + gap->m_count;
          if(m_requested < end) {
            pending = CxaPitchGap(m_requested, end - m_requested);
            position = m_live;
            m_requested = end;
          }
        });
        if(pending && m_gap) {
          auto count = (*m_gap)->request(m_unit, *pending, position, timestamp);
          if(count != pending->m_count) {
            Beam::with(m_sequencer, [&] (auto&) {
              if(m_requested == pending->m_sequence + pending->m_count) {
                m_requested = pending->m_sequence + count;
              }
            });
          }
        }
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void CxaPitchClient<P, G, S, R>::recovery_loop(int index) {
    while(true) {
      try {
        auto block = m_recovery[index]->read();
        if(block.get_header().m_unit != m_unit) {
          continue;
        }
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          sequencer.recover(block);
          flush(sequencer);
          if(!sequencer.get_gap()) {
            m_reported_gap = 0;
          }
        });
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void CxaPitchClient<P, G, S, R>::response_loop() {
    while(true) {
      try {
        auto response = (*m_gap)->get_responses()->pop();
        if(response.m_status == CxaPitchGapResponse::ACCEPTED) {
          continue;
        }
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          auto gap = sequencer.get_gap();
          if(!gap || gap->m_sequence != response.m_sequence) {
            return;
          }
          std::cout << "(dropped " << m_time_client->get_time() << ' ' <<
            response.m_sequence << ' ' << response.m_count << ')' <<
            std::endl;
          sequencer.reset(response.m_sequence + response.m_count);
          m_reported_gap = 0;
          flush(sequencer);
        });
      } catch(const std::exception&) {
        break;
      }
    }
  }

  template<typename P, typename G, typename S, typename R>
    requires IsCxaPitchProtocolClient<Beam::dereference_t<P>> &&
      IsCxaPitchGapClient<Beam::dereference_t<G>> &&
      IsCxaPitchSpinClient<Beam::dereference_t<S>> &&
      Beam::IsTimeClient<Beam::dereference_t<R>>
  void CxaPitchClient<P, G, S, R>::spin_loop() {
    auto attempts = 0;
    try {
      while(attempts != SPIN_ATTEMPTS) {
        auto offer = (*m_spin)->get_offers()->pop();
        auto sequence = boost::optional<std::uint32_t>();
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          sequence = sequencer.get_sequence();
        });
        if(!sequence || offer + 1 < *sequence) {
          continue;
        }
        auto spin = (*m_spin)->request(offer);
        if(spin.m_status != CxaPitchSpinResponse::ACCEPTED) {
          ++attempts;
          continue;
        }
        Beam::with(m_sequencer, [&] (auto& sequencer) {
          for(auto& message : spin.m_messages) {
            m_messages->push(message);
          }
          sequencer.reset(spin.m_sequence + 1);
          m_is_ready = true;
          flush(sequencer);
        });
        break;
      }
    } catch(const std::exception&) {}
    Beam::with(m_sequencer, [&] (auto& sequencer) {
      if(!m_is_ready) {
        m_is_ready = true;
        flush(sequencer);
      }
    });
    (*m_spin)->close();
  }
}

#endif
