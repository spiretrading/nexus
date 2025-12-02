#ifndef OASIS_SERENITY_FIX_APPLICATION_HPP
#define OASIS_SERENITY_FIX_APPLICATION_HPP
#include <unordered_map>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <quickfix/MessageCracker.h>
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/FixUtilities/FixOrderLog.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"

namespace Nexus {

  /** Implements a FIX Application for the Serenity FIX Server. */
  class SerenityFixApplication final :
      public FixApplication, public FIX::MessageCracker {
    public:

      /**
       * Constructs a SerenityFixApplication.
       * @param time_client The TimeClient used for timestamps.
       * @param market_data_client The MarketDataClient used to determine the
       *        BBO.
       */
      SerenityFixApplication(Beam::Ref<Beam::LiveNtpTimeClient> time_client,
        Beam::Ref<ApplicationMarketDataClient> market_data_client);

      std::shared_ptr<Order> recover(
        const SequencedAccountOrderRecord& record) override;
      std::shared_ptr<Order> submit(const OrderInfo& info) override;
      void cancel(const OrderExecutionSession& session, OrderId id) override;
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report) override;
      void onCreate(const FIX::SessionID&) override;
      void onLogon(const FIX::SessionID& sessionID) override;
      void onLogout(const FIX::SessionID& sessionID) override;
      void toAdmin(FIX::Message&, const FIX::SessionID&) override;
      void toApp(FIX::Message&, const FIX::SessionID&) override;
      void fromAdmin(const FIX::Message&, const FIX::SessionID&) override;
      void fromApp(
        const FIX::Message& message, const FIX::SessionID& sessionID) override;
      void onMessage(const FIX42::ExecutionReport& message,
        const FIX::SessionID& session_id) override;
      void onMessage(const FIX42::TradingSessionStatus& message,
        const FIX::SessionID& session_id) override;
      void onMessage(const FIX42::OrderCancelReject& message,
        const FIX::SessionID& session_id) override;

    private:
      Beam::LiveNtpTimeClient* m_time_client;
      ApplicationMarketDataClient* m_market_data_client;
      Beam::SynchronizedUnorderedMap<
        Security, std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bbo_quotes;
      mutable boost::optional<boost::optional<std::string>> m_anonymous_tag;
      Beam::SynchronizedUnorderedSet<OrderId> m_cancellations;
      FixOrderLog m_order_log;

      BboQuote load_bbo_quote(const Security& security);
      void route_to_chix(const Nexus::OrderInfo& info,
        Beam::Out<FIX42::NewOrderSingle> new_order_single);
      void route_to_cse(const Nexus::OrderInfo& info,
        Beam::Out<FIX42::NewOrderSingle> new_order_single);
      void route_to_matn(const Nexus::OrderInfo& info,
        Beam::Out<FIX42::NewOrderSingle> new_order_single);
      void route_to_neo(const Nexus::OrderInfo& info,
        Beam::Out<FIX42::NewOrderSingle> new_order_single);
      void route_to_tsx(const Nexus::OrderInfo& info,
        Beam::Out<FIX42::NewOrderSingle> new_order_single);
      const boost::optional<std::string>& get_anonymous_tag() const;
      std::string get_umir_user_id() const;
      std::string get_no_trade_feat() const;
      std::string get_no_trade_key() const;
  };
}

#endif
