#ifndef CXA_PITCH_SPIN_CLIENT_HPP
#define CXA_PITCH_SPIN_CLIENT_HPP
#include <atomic>
#include <concepts>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionMessages.hpp"

namespace Nexus {

  /** Stores a snapshot of the open orders on a unit. */
  struct CxaPitchSpin {

    /** The sequence that the snapshot is current through. */
    std::uint32_t m_sequence;

    /** Whether the request was accepted, or the reason it was rejected. */
    char m_status;

    /** The messages making up the snapshot. */
    std::vector<Beam::SharedBuffer> m_messages;
  };

  /** Concept satisfied by types requesting a snapshot of a unit's book. */
  template<typename T>
  concept IsCxaPitchSpinClient = requires(T& t) {
    { std::as_const(t).get_progress() } -> std::same_as<std::uint64_t>;
    { t.get_offers() } -> std::convertible_to<
      const std::shared_ptr<Beam::Queue<std::uint32_t>>&>;
    { t.request(std::declval<std::uint32_t>()) } -> std::same_as<CxaPitchSpin>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Requests a snapshot of a unit's open orders from a CXA PITCH spin server.
   * @param <S> The type of session connected to the spin server.
   */
  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  class CxaPitchSpinClient {
    public:

      /** The type of session connected to the spin server. */
      using Session = Beam::dereference_t<S>;

      /**
       * Constructs a CxaPitchSpinClient.
       * @param session The session connected to the spin server.
       */
      template<Beam::Initializes<S> SF>
      explicit CxaPitchSpinClient(SF&& session);

      ~CxaPitchSpinClient();

      /** Returns the queue of sequences a snapshot is available through. */
      const std::shared_ptr<Beam::Queue<std::uint32_t>>& get_offers() const;

      /** Returns the number of snapshot messages received by this client. */
      std::uint64_t get_progress() const;

      /**
       * Requests a snapshot of the open orders on this session's unit.
       * @param sequence The sequence, taken from an offer, to request the
       *        snapshot at.
       * @return The snapshot, or the reason that the request was rejected.
       */
      CxaPitchSpin request(std::uint32_t sequence);

      /** Closes the connection to the spin server. */
      void close();

    private:
      Beam::local_ptr_t<S> m_session;
      std::shared_ptr<Beam::Queue<std::uint32_t>> m_offers;
      std::shared_ptr<Beam::Queue<CxaPitchSpin>> m_spins;
      CxaPitchSpin m_spin;
      bool m_is_spinning;
      std::atomic_uint64_t m_progress;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      CxaPitchSpinClient(const CxaPitchSpinClient&) = delete;
      CxaPitchSpinClient& operator =(const CxaPitchSpinClient&) = delete;
      void read_loop();
  };

  template<typename SF>
  CxaPitchSpinClient(SF&&) -> CxaPitchSpinClient<std::remove_cvref_t<SF>>;

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  template<Beam::Initializes<S> SF>
  CxaPitchSpinClient<S>::CxaPitchSpinClient(SF&& session)
      try : m_session(std::forward<SF>(session)),
            m_offers(std::make_shared<Beam::Queue<std::uint32_t>>()),
            m_spins(std::make_shared<Beam::Queue<CxaPitchSpin>>()),
            m_is_spinning(false),
            m_progress(0) {
    m_read_loop =
      Beam::spawn(std::bind_front(&CxaPitchSpinClient::read_loop, this));
  } catch(const std::exception&) {
    Beam::throw_nested_with_location(Beam::ConnectException(
      "Failed to initialize the CXA PITCH spin client."));
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  CxaPitchSpinClient<S>::~CxaPitchSpinClient() {
    close();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  const std::shared_ptr<Beam::Queue<std::uint32_t>>&
      CxaPitchSpinClient<S>::get_offers() const {
    return m_offers;
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  std::uint64_t CxaPitchSpinClient<S>::get_progress() const {
    return m_progress.load();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  CxaPitchSpin CxaPitchSpinClient<S>::request(std::uint32_t sequence) {
    auto message = CxaPitchSpinRequest();
    message.m_sequence = sequence;
    m_session->write(message);
    return m_spins->pop();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  void CxaPitchSpinClient<S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_session->close();
    m_offers->close();
    m_spins->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  void CxaPitchSpinClient<S>::read_loop() {
    try {
      while(true) {
        auto message = m_session->read();
        if(message.m_type == CxaPitchSpinImageAvailable::TYPE) {
          m_offers->push(CxaPitchSpinImageAvailable::parse(message).m_sequence);
        } else if(message.m_type == CxaPitchSpinResponse::TYPE) {
          auto response = CxaPitchSpinResponse::parse(message);
          m_spin = CxaPitchSpin();
          m_spin.m_sequence = response.m_sequence;
          m_spin.m_status = response.m_status;
          m_is_spinning = response.m_status == CxaPitchSpinResponse::ACCEPTED;
          if(!m_is_spinning) {
            m_spins->push(std::move(m_spin));
          }
        } else if(message.m_type == CxaPitchSpinFinished::TYPE) {
          if(m_is_spinning) {
            m_is_spinning = false;
            m_spins->push(std::move(m_spin));
          }
        } else if(m_is_spinning) {
          validate(message);
          m_spin.m_messages.emplace_back(
            message.m_payload - CxaPitchMessage::HEADER_LENGTH,
            message.m_length);
          ++m_progress;
        }
      }
    } catch(const std::exception&) {
      m_offers->close(std::current_exception());
      m_spins->close(std::current_exception());
    }
  }
}

#endif
