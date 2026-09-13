#ifndef CXA_PITCH_SPIN_CLIENT_HPP
#define CXA_PITCH_SPIN_CLIENT_HPP
#include <exception>
#include <functional>
#include <vector>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchMessages.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionClient.hpp"

namespace Nexus {

  /** Stores a snapshot of the open orders on a feed. */
  struct CxaPitchSnapshot {

    /** The sequence that the snapshot is current through. */
    std::uint32_t m_sequence;

    /** Whether the request was accepted, or the reason it was rejected. */
    char m_status;

    /** The messages making up the snapshot. */
    std::vector<Beam::SharedBuffer> m_messages;
  };

  /** Concept satisfied by types requesting a snapshot of a feed's book. */
  template<typename T>
  concept IsCxaPitchSpinClient = requires(T& t) {
    { std::as_const(t).monitor_snapshot_sequences(
        std::declval<Beam::ScopedQueueWriter<std::uint32_t>>()) } ->
      std::same_as<void>;
    { t.load_snapshot(std::declval<std::uint32_t>()) } ->
      std::same_as<CxaPitchSnapshot>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Requests a snapshot of a feed's open orders from a CXA PITCH spin server.
   * @tparam S The type of session connected to the spin server.
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

      /**
       * Monitors the latest available snapshot sequence and subsequent updates.
       * @param queue Receives the snapshot sequences.
       */
      void monitor_snapshot_sequences(
        Beam::ScopedQueueWriter<std::uint32_t> queue) const;

      /**
       * Loads a snapshot of the open orders on this session's feed.
       * @param sequence The sequence to request the snapshot at.
       * @return The snapshot or the reason that the request was rejected.
       */
      CxaPitchSnapshot load_snapshot(std::uint32_t sequence);

      /** Closes the connection to the spin server. */
      void close();

    private:
      Beam::local_ptr_t<S> m_session;
      Beam::StatePublisher<std::uint32_t> m_sequences;
      Beam::Queue<CxaPitchSnapshot> m_snapshots;
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
      try : m_session(std::forward<SF>(session)) {
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
  void CxaPitchSpinClient<S>::monitor_snapshot_sequences(
      Beam::ScopedQueueWriter<std::uint32_t> queue) const {
    m_sequences.monitor(std::move(queue));
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  CxaPitchSnapshot CxaPitchSpinClient<S>::load_snapshot(
      std::uint32_t sequence) {
    auto message = CxaPitchSpinRequest(sequence);
    m_session->write(message);
    return m_snapshots.pop();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  void CxaPitchSpinClient<S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_snapshots.close();
    m_sequences.close();
    m_session->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename S> requires IsCxaPitchSessionClient<Beam::dereference_t<S>>
  void CxaPitchSpinClient<S>::read_loop() {
    try {
      auto snapshot = CxaPitchSnapshot();
      auto orders = std::uint32_t(0);
      while(true) {
        auto message = m_session->read();
        if(message.m_type == CxaPitchSpinImageAvailable::TYPE) {
          m_sequences.push(
            CxaPitchSpinImageAvailable::parse(message).m_sequence);
        } else if(message.m_type == CxaPitchSpinResponse::TYPE) {
          auto response = CxaPitchSpinResponse::parse(message);
          snapshot = CxaPitchSnapshot();
          snapshot.m_sequence = response.m_sequence;
          snapshot.m_status = response.m_status;
          orders = response.m_order_count;
          if(response.m_status != CxaPitchSpinResponse::ACCEPTED) {
            m_snapshots.push(std::move(snapshot));
          }
        } else if(message.m_type == CxaPitchSpinFinished::TYPE) {
          auto finished = CxaPitchSpinFinished::parse(message);
          if(finished.m_sequence != snapshot.m_sequence) {
            boost::throw_with_location(
              CxaPitchParserException("Spin finished out of sequence."));
          }
          if(orders != 0) {
            boost::throw_with_location(
              CxaPitchParserException("Spin order count mismatch."));
          }
          m_snapshots.push(std::move(snapshot));
        } else {
          validate(message);
          if(message.m_type == CxaPitchAddOrder::TYPE) {
            if(orders == 0) {
              boost::throw_with_location(
                CxaPitchParserException("Spin order count mismatch."));
            }
            --orders;
          }
          snapshot.m_messages.emplace_back(
            message.m_payload - CxaPitchMessage::HEADER_LENGTH,
            message.m_length);
        }
      }
    } catch(const std::exception&) {
      m_sequences.close(std::current_exception());
      m_snapshots.close(std::current_exception());
    }
  }
}

#endif
