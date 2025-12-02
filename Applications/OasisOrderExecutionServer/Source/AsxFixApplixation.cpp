#include "OasisOrderExecutionServer/AsxFixApplication.hpp"
#include <boost/throw_exception.hpp>
#include <quickfix/Session.h>
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FixUtilities/FixConversions.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  const auto ACCOUNT_TAG = 28888;
}

AsxFixApplication::AsxFixApplication(Ref<LiveNtpTimeClient> time_client)
  : m_time_client(time_client.get()) {}

std::shared_ptr<Order> AsxFixApplication::recover(
    const SequencedAccountOrderRecord& record) {
  return m_order_log.recover(record);
}

std::shared_ptr<Order> AsxFixApplication::submit(const OrderInfo& info) {
  return m_order_log.submit(info, get_session_id().getSenderCompID(),
    get_session_id().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> new_order_single) {
      if(info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::GTC ||
          info.m_fields.m_time_in_force.get_type() == TimeInForce::Type::GTD) {
        throw_with_location(
          FixOrderRejectedException("Invalid time in force."));
      }
      if(info.m_fields.m_type == OrderType::STOP) {
        throw_with_location(FixOrderRejectedException("Invalid order type."));
      }
      if(info.m_fields.m_currency != DefaultCurrencies::AUD) {
        throw_with_location(FixOrderRejectedException("Invalid currency."));
      }
      new_order_single->set(FIX::Account(get_account()));
      new_order_single->setField(ACCOUNT_TAG, info.m_submission_account.m_name);
      if(auto deliver_to_comp_id = get_deliver_to_comp_id()) {
        new_order_single->getHeader().setField(
          FIX::DeliverToCompID(*deliver_to_comp_id));
      }
      if(info.m_fields.m_security.get_venue() == DefaultVenues::ASX) {
        new_order_single->set(FIX::SecurityExchange("ASX"));
      } else {
        throw_with_location(FixOrderRejectedException("Invalid venue."));
      }
      if(info.m_fields.m_destination == DefaultDestinations::ASXT) {
        new_order_single->set(FIX::ExDestination("BESTMKT"));
      } else if(info.m_fields.m_destination == DefaultDestinations::CXA) {
        if(info.m_fields.m_type == OrderType::PEGGED) {
          new_order_single->set(FIX::ExDestination("CXA"));
        } else {
          new_order_single->set(FIX::ExDestination("BESTMKT2"));
        }
      } else {
        throw_with_location(FixOrderRejectedException("Invalid destination."));
      }
      if(info.m_fields.m_type == OrderType::MARKET) {
        new_order_single->set(FIX::OrdType('K'));
      }
    });
}

void AsxFixApplication::cancel(
    const OrderExecutionSession& session, OrderId id) {
  m_order_log.cancel(session, id, m_time_client->get_time(),
    get_session_id().getSenderCompID(), get_session_id().getTargetCompID(),
    [&] (const std::shared_ptr<Order>& order,
        Out<FIX42::OrderCancelRequest> request) {
      request->set(FIX::Account(get_account()));
      request->setField(ACCOUNT_TAG, session.get_account().m_name);
      if(auto deliver_to_comp_id = get_deliver_to_comp_id()) {
        request->getHeader().setField(
          FIX::DeliverToCompID(*deliver_to_comp_id));
      }
      auto& fields = order->get_info().m_fields;
      if(fields.m_security.get_venue() == DefaultVenues::ASX) {
        request->set(FIX::SecurityExchange("ASX"));
      } else {
        throw_with_location(FixOrderRejectedException("Invalid venue."));
      }
    });
}

void AsxFixApplication::update(const OrderExecutionSession& session,
    OrderId id, const ExecutionReport& report) {
  m_order_log.update(session, id, report, m_time_client->get_time());
}

void AsxFixApplication::onCreate(const FIX::SessionID&) {}

void AsxFixApplication::onLogon(const FIX::SessionID&) {}

void AsxFixApplication::onLogout(const FIX::SessionID&) {}

void AsxFixApplication::toAdmin(FIX::Message&, const FIX::SessionID&) {}

void AsxFixApplication::toApp(FIX::Message&, const FIX::SessionID&) {}

void AsxFixApplication::fromAdmin(const FIX::Message&, const FIX::SessionID&) {}

void AsxFixApplication::fromApp(
    const FIX::Message& message, const FIX::SessionID& session_id) {
  crack(message, session_id);
}

void AsxFixApplication::onMessage(
    const FIX42::ExecutionReport& message, const FIX::SessionID& session_id) {
  m_order_log.update(message, session_id, m_time_client->get_time(),
    [=] (const std::shared_ptr<Order>& order, Out<ExecutionReport> update) {
      if(update->m_last_quantity != 0) {
        update->m_liquidity_flag =
          lexical_cast<std::string>(LiquidityFlag::ACTIVE);
        auto last_mkt = FIX::LastMkt();
        if(message.isSet(last_mkt)) {
          message.get(last_mkt);
        }
        if(last_mkt == "CXA" || last_mkt == "CXAP" || last_mkt == "CXAC") {
          update->m_last_market = DefaultDestinations::CXA;
        } else if(last_mkt == "TM") {
          update->m_last_market = DefaultDestinations::ASXT;
        } else {
          update->m_last_market = order->get_info().m_fields.m_destination;
        }
      }
    });
}

void AsxFixApplication::onMessage(
  const FIX42::TradingSessionStatus&, const FIX::SessionID&) {}

void AsxFixApplication::onMessage(
  const FIX42::OrderCancelReject&, const FIX::SessionID&) {}

std::string AsxFixApplication::get_account() const {
  return get_session_settings().get(get_session_id()).getString("Account");
}

std::string AsxFixApplication::get_username() const {
  return get_session_settings().get(get_session_id()).getString("Username");
}

std::string AsxFixApplication::get_password() const {
  return get_session_settings().get(get_session_id()).getString("Password");
}

optional<std::string> AsxFixApplication::get_deliver_to_comp_id() const {
  auto& settings = get_session_settings().get(get_session_id());
  if(settings.has("DeliverToCompID")) {
    return settings.getString("DeliverToCompID");
  }
  return none;
}
