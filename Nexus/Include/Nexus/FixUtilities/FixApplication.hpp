#ifndef NEXUS_FIX_APPLICATION_HPP
#define NEXUS_FIX_APPLICATION_HPP
#include <boost/date_time/posix_time/ptime.hpp>
#include <quickfix/Application.h>
#include "Nexus/FixUtilities/FixUtilities.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::FixUtilities {

  /** Implements a FIX Application to be used by the FixOrderExecutionDriver. */
  class FixApplication : public FIX::Application {
    public:

      /** Returns the session id. */
      const FIX::SessionID& GetSessionId() const;

      /** Returns the session settings. */
      const FIX::SessionSettings& GetSessionSettings() const;

      virtual const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord&
        orderRecord) = 0;

      virtual const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& info) = 0;

      virtual void Cancel(
        const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId) = 0;

      virtual void Update(
        const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport) = 0;

    protected:

      /**
       * Sets the application's session settings.
       * @param sessionId The application's session id.
       * @param sessionSettings The session's settings.
       */
      void SetSessionSettings(const FIX::SessionID& sessionId,
        const FIX::SessionSettings& sessionSettings);

    private:
      friend class FixOrderExecutionDriver;
      FIX::SessionID m_sessionId;
      FIX::SessionSettings m_sessionSettings;
  };

  inline const FIX::SessionID& FixApplication::GetSessionId() const {
    return m_sessionId;
  }

  inline const FIX::SessionSettings& FixApplication::
      GetSessionSettings() const {
    return m_sessionSettings;
  }

  inline void FixApplication::SetSessionSettings(
      const FIX::SessionID& sessionId,
      const FIX::SessionSettings& sessionSettings) {
    m_sessionId = sessionId;
    m_sessionSettings = sessionSettings;
  }
}

#endif
