#ifndef OASIS_ASX_FIX_APPLICATION_HPP
#define OASIS_ASX_FIX_APPLICATION_HPP
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <boost/optional/optional.hpp>
#include <quickfix/MessageCracker.h>
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/FixUtilities/FixOrderLog.hpp"

namespace Nexus {

  /** Implements a FIX Application for OpenMarket's ASX FIX Server. */
  class AsxFixApplication : public FixApplication, public FIX::MessageCracker {
    public:

      /**
       * Constructs an AsxFixApplication.
       * @param time_client The TimeClient used for timestamps.
       */
      explicit AsxFixApplication(
        Beam::Ref<Beam::LiveNtpTimeClient> time_client);

      std::shared_ptr<Order> recover(
        const SequencedAccountOrderRecord& record) override;
      std::shared_ptr<Order> submit(const OrderInfo& info) override;
      void cancel(const OrderExecutionSession& session, OrderId id) override;
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report) override;
      void onCreate(const FIX::SessionID&) override;
      void onLogon(const FIX::SessionID& session_id) override;
      void onLogout(const FIX::SessionID& session_id) override;
      void toAdmin(FIX::Message&, const FIX::SessionID&) override;
      void toApp(FIX::Message&, const FIX::SessionID&) override;
      void fromAdmin(const FIX::Message&, const FIX::SessionID&) override;
      void fromApp(
        const FIX::Message& message, const FIX::SessionID& session_id) override;
      void onMessage(const FIX42::ExecutionReport& message,
        const FIX::SessionID& session_id) override;
      void onMessage(const FIX42::TradingSessionStatus& message,
        const FIX::SessionID& session_id) override;
      void onMessage(const FIX42::OrderCancelReject& message,
        const FIX::SessionID& session_id) override;

    private:
      Beam::LiveNtpTimeClient* m_time_client;
      FixOrderLog m_order_log;

      std::string get_account() const;
      std::string get_username() const;
      std::string get_password() const;
      boost::optional<std::string> get_deliver_to_comp_id() const;
  };
}

#endif
