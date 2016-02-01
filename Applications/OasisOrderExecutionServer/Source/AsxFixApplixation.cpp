#include "OasisOrderExecutionServer/AsxFixApplication.hpp"
#include <boost/throw_exception.hpp>
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
using namespace std;

namespace {
  const auto ACCOUNT_TAG = 28888;
}

AsxFixApplication::AsxFixApplication(RefType<LiveNtpTimeClient> timeClient)
    : m_timeClient(timeClient.Get()) {}

const Order& AsxFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& AsxFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      newOrderSingle->set(FIX::Account(GetAccount()));
      newOrderSingle->setField(ACCOUNT_TAG, info.m_submissionAccount.m_name);
      if(info.m_fields.m_security.GetMarket() == DefaultMarkets::ASX()) {
        newOrderSingle->set(FIX::SecurityExchange{"ASX"});
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid market."});
      }
      if(info.m_fields.m_destination == DefaultDestinations::ASXT()) {
        newOrderSingle->set(FIX::ExDestination{"ASX"});
      } else if(info.m_fields.m_destination == DefaultDestinations::CXA()) {
        newOrderSingle->set(FIX::ExDestination{"CXA"});
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{
          "Invalid destination."});
      }
      if(info.m_fields.m_type == OrderType::MARKET) {
        newOrderSingle->set(FIX::OrdType{'K'});
      }
    });
}

void AsxFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  auto order = m_orderLog.FindOrder(orderId);
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->set(FIX::Account(GetAccount()));
      orderCancelRequest->setField(ACCOUNT_TAG, session.GetAccount().m_name);
      if(order != nullptr) {
        auto& fields = order->GetInfo().m_fields;
        if(fields.m_security.GetMarket() == DefaultMarkets::ASX()) {
          orderCancelRequest->set(FIX::SecurityExchange{"ASX"});
        } else {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid market."});
        }
      }
    });
}

void AsxFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport, m_timeClient->GetTime());
}

void AsxFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void AsxFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void AsxFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void AsxFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void AsxFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::DoNotSend) {}

void AsxFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) {}

void AsxFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
    FIX::UnsupportedMessageType) {
  crack(message, sessionID);
}

void AsxFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  auto orderId = FixOrderLog::GetOrderId(message);
  std::shared_ptr<Order> order;
  if(orderId.is_initialized()) {
    order = m_orderLog.FindOrder(*orderId);
  }
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (Out<ExecutionReport> update) {
      if(update->m_lastQuantity != 0) {
        update->m_liquidityFlag = ToString(LiquidityFlag::ACTIVE);
        if(order != nullptr) {
          update->m_lastMarket = order->GetInfo().m_fields.m_destination;
        }
      }
    });
}

void AsxFixApplication::onMessage(const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void AsxFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

string AsxFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

string AsxFixApplication::GetUsername() const {
  return GetSessionSettings().get(GetSessionId()).getString("Username");
}

string AsxFixApplication::GetPassword() const {
  return GetSessionSettings().get(GetSessionId()).getString("Password");
}
