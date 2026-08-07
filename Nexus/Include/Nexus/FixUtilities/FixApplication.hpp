#ifndef NEXUS_FIX_APPLICATION_HPP
#define NEXUS_FIX_APPLICATION_HPP
#include <quickfix/Application.h>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"

namespace Nexus {

  /** Implements a FIX Application to be used by the FixOrderExecutionDriver. */
  class FixApplication : public FIX::Application {
    public:

      /** Returns the session id. */
      const FIX::SessionID& get_session_id() const;

      /** Returns the session settings. */
      const FIX::SessionSettings& get_session_settings() const;

      /**
       * Recovers a previously submitted Order.
       * @param record The OrderRecord to recover.
       * @return The recovered Order.
       */
      virtual std::shared_ptr<Order> recover(
        const SequencedAccountOrderRecord& record) = 0;

      /**
       * Submits an Order.
       * @param info The OrderInfo containing the details of the submission.
       * @return The Order that was submitted.
       */
      virtual std::shared_ptr<Order> submit(const OrderInfo& info) = 0;

      /**
       * Cancels an Order.
       * @param session The session requesting the cancel.
       * @param id The id of the Order to cancel.
       */
      virtual void cancel(const OrderExecutionSession& session, OrderId id) = 0;

      /**
       * Updates an Order with an ExecutionReport.
       * @param session The session requesting the update.
       * @param id The id of the Order to update.
       * @param report The ExecutionReport containing the update.
       */
      virtual void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report) = 0;

    protected:

      /**
       * Sets the application's session settings.
       * @param session_id The application's session id.
       * @param session_settings The session's settings.
       */
      void set_session_settings(const FIX::SessionID& session_id,
        const FIX::SessionSettings& session_settings);

    private:
      friend class FixOrderExecutionDriver;
      FIX::SessionID m_session_id;
      FIX::SessionSettings m_session_settings;
  };

  inline const FIX::SessionID& FixApplication::get_session_id() const {
    return m_session_id;
  }

  inline const FIX::SessionSettings&
      FixApplication::get_session_settings() const {
    return m_session_settings;
  }

  inline void FixApplication::set_session_settings(
      const FIX::SessionID& session_id,
      const FIX::SessionSettings& session_settings) {
    m_session_id = session_id;
    m_session_settings = session_settings;
  }
}

#endif
