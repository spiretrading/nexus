#ifndef OASIS_ASX_FIX_APPLICATION_HPP
#define OASIS_ASX_FIX_APPLICATION_HPP
#include <Beam/Network/Network.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <boost/optional/optional.hpp>
#include <quickfix/MessageCracker.h>
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/FixUtilities/FixOrderLog.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OasisOrderExecutionService {

  /** Implements a FIX Application for OpenMarket's ASX FIX Server. */
  class AsxFixApplication : public FixUtilities::FixApplication,
      public FIX::MessageCracker {
    public:

      /**
       * Constructs an AsxFixApplication.
       * @param timeClient The TimeClient used for timestamps.
       */
      AsxFixApplication(Beam::Ref<Beam::TimeService::LiveNtpTimeClient>
        timeClient);

      const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& orderRecord)
        override;

      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& info) override;

      void Cancel(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId) override;

      void Update(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport) override;

      void onCreate(const FIX::SessionID&) override;

      void onLogon(const FIX::SessionID& sessionID) override;

      void onLogout(const FIX::SessionID& sessionID) override;

      void toAdmin(FIX::Message&, const FIX::SessionID&) override;

      void toApp(FIX::Message&, const FIX::SessionID&) override;

      void fromAdmin(const FIX::Message&, const FIX::SessionID&) override;

      void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID)
        override;

      void onMessage(const FIX42::ExecutionReport& message,
        const FIX::SessionID& sessionId) override;

      void onMessage(const FIX42::TradingSessionStatus& message,
        const FIX::SessionID& sessionId) override;

      void onMessage(const FIX42::OrderCancelReject& message,
        const FIX::SessionID& sessionId) override;

    private:
      Beam::TimeService::LiveNtpTimeClient* m_timeClient;
      FixUtilities::FixOrderLog m_orderLog;

      std::string GetAccount() const;
      std::string GetUsername() const;
      std::string GetPassword() const;
      boost::optional<std::string> GetDeliverToCompId() const;
  };
}

#endif
