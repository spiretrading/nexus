#ifndef OASIS_LEKFIXAPPLICATION_HPP
#define OASIS_LEKFIXAPPLICATION_HPP
#include <Beam/Network/Network.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <boost/optional/optional.hpp>
#include <quickfix/MessageCracker.h>
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/FixUtilities/FixOrderLog.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OasisOrderExecutionService {

  /*! \class LekFixApplication
      \brief Implements a FIX Application for the LEK Securities brokerage.
   */
  class LekFixApplication : public FixUtilities::FixApplication,
      public FIX::MessageCracker {
    public:

      //! Constructs an LekFixApplication.
      /*!
        \param timeClient The TimeClient used for timestamps.
      */
      LekFixApplication(Beam::RefType<Beam::TimeService::LiveNtpTimeClient>
        timeClient);

      virtual const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& orderRecord);

      virtual const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& info);

      virtual void Cancel(
        const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId);

      virtual void Update(
        const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport);

      virtual void onCreate(const FIX::SessionID&);

      virtual void onLogon(const FIX::SessionID& sessionID);

      virtual void onLogout(const FIX::SessionID& sessionID);

      virtual void toAdmin(FIX::Message&, const FIX::SessionID&);

      virtual void toApp(FIX::Message&, const FIX::SessionID&);

      virtual void fromAdmin(const FIX::Message&, const FIX::SessionID&);

      virtual void fromApp(const FIX::Message& message,
        const FIX::SessionID& sessionID);

      virtual void onMessage(const FIX42::ExecutionReport& message,
        const FIX::SessionID& sessionId);

      virtual void onMessage(const FIX42::TradingSessionStatus& message,
        const FIX::SessionID& sessionId);

      virtual void onMessage(const FIX42::OrderCancelReject& message,
        const FIX::SessionID& sessionId);

    private:
      Beam::TimeService::LiveNtpTimeClient* m_timeClient;
      FixUtilities::FixOrderLog m_orderLog;
      mutable boost::optional<boost::optional<std::string>> m_accountTag;

      const boost::optional<std::string>& GetAccount() const;
  };
}
}

#endif
