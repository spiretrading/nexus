#include "OasisOrderExecutionServer/MonexBoomFixApplication.hpp"
#include <boost/throw_exception.hpp>
#include <quickfix/fix44/Logon.h>
#include <quickfix/Session.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/FixUtilities/FixConversions.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::FixUtilities;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;

MonexBoomFixApplication::MonexBoomFixApplication(
  Ref<LiveNtpTimeClient> timeClient)
  : m_timeClient(timeClient.Get()) {}

const Order& MonexBoomFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& MonexBoomFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX44::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_destination == DefaultDestinations::HKEX()) {
        if(info.m_fields.m_security.GetCountry() != DefaultCountries::HK()) {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid country."));
        }
        if(info.m_fields.m_currency != DefaultCurrencies::HKD()) {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid currency."));
        }
        newOrderSingle->set(FIX::SecurityExchange("XHKG"));
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException(
          "Invalid destination."));
      }
      newOrderSingle->set(GetAccount());
      if(info.m_fields.m_type == OrderType::MARKET) {
        newOrderSingle->set(FIX::OrdType('2'));
        if(info.m_fields.m_side == Side::ASK) {
          newOrderSingle->set(FIX::Price(0.01));
        } else {
          newOrderSingle->set(FIX::Price(10000));
        }
      }
    });
}

void MonexBoomFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX44::OrderCancelRequest> orderCancelRequest) {
      auto& fields = order.GetInfo().m_fields;
      if(fields.m_security.GetMarket() == DefaultMarkets::HKEX()) {
        orderCancelRequest->set(FIX::SecurityExchange("XHKG"));
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid market."));
      }
      orderCancelRequest->set(
        FIX::Symbol(order.GetInfo().m_fields.m_security.GetSymbol()));
      orderCancelRequest->set(GetAccount());
    });
}

void MonexBoomFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport, m_timeClient->GetTime());
}

void MonexBoomFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void MonexBoomFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void MonexBoomFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void MonexBoomFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {
  if(message.getHeader().getField(FIX::FIELD::MsgType) == FIX::MsgType_Logon) {
    auto& logon = static_cast<FIX44::Logon&>(message);
    logon.setField(GetUsername());
    logon.setField(GetPassword());
  }
}

void MonexBoomFixApplication::toApp(FIX::Message& message,
  const FIX::SessionID& sessionID) {}

void MonexBoomFixApplication::fromAdmin(const FIX::Message& message,
  const FIX::SessionID& sessionID) {}

void MonexBoomFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) {
  crack(message, sessionID);
}

void MonexBoomFixApplication::onMessage(const FIX44::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      if(update->m_lastQuantity != 0) {
        update->m_liquidityFlag = ToString(LiquidityFlag::ACTIVE);
        update->m_lastMarket = order.GetInfo().m_fields.m_destination;
      }
    });
}

void MonexBoomFixApplication::onMessage(
  const FIX44::TradingSessionStatus& message,
  const FIX::SessionID& sessionId) {}

void MonexBoomFixApplication::onMessage(const FIX44::OrderCancelReject& message,
  const FIX::SessionID& sessionId) {}

FIX::Account MonexBoomFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

FIX::Username MonexBoomFixApplication::GetUsername() const {
  return GetSessionSettings().get(GetSessionId()).getString("Username");
}

FIX::Password MonexBoomFixApplication::GetPassword() const {
  return GetSessionSettings().get(GetSessionId()).getString("Password");
}
