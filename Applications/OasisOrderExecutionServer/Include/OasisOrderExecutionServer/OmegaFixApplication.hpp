#ifndef OASIS_OMEGA_FIX_APPLICATION_HPP
#define OASIS_OMEGA_FIX_APPLICATION_HPP
#include <unordered_map>
#include <Beam/Network/Network.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <quickfix/MessageCracker.h>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/FixUtilities/FixOrderLog.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OasisOrderExecutionService {

  /* Implements a FIX Application using the Omega ATS. */
  class OmegaFixApplication : public FixUtilities::FixApplication,
      public FIX::MessageCracker {
    public:

      //! Constructs an OmegaFixApplication.
      /*!
        \param timeClient The TimeClient used for timestamps.
        \param marketDataClient The MarketDataClient used to determine the BBO.
      */
      OmegaFixApplication(Beam::Ref<Beam::TimeService::LiveNtpTimeClient>
        timeClient,
        Beam::Ref<MarketDataService::ApplicationMarketDataClient::Client>
        marketDataClient);

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
      MarketDataService::ApplicationMarketDataClient::Client*
        m_marketDataClient;
      Beam::SynchronizedMap<std::unordered_map<
        Security, std::shared_ptr<Beam::StateQueue<BboQuote>>>> m_bboQuotes;
      FixUtilities::FixOrderLog m_orderLog;
      mutable boost::optional<boost::optional<std::string>> m_anonymousTag;

      BboQuote LoadBboQuote(const Security& security);
      std::string GetAccount() const;
      std::string GetExecBroker() const;
      std::string GetUmirUserID() const;
      std::string GetOmegaRoute() const;
      std::string GetLynxRoute() const;
      const boost::optional<std::string>& GetAnonymousTag() const;
      std::string GetNoTradeFeat() const;
      std::string GetNoTradeKey() const;
  };
}

#endif
