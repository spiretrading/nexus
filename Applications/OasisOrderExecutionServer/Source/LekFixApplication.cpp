#include "OasisOrderExecutionServer/LekFixApplication.hpp"
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/throw_exception.hpp>
#include <quickfix/Session.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
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

LekFixApplication::LekFixApplication(RefType<LiveNtpTimeClient> timeClient)
    : m_timeClient(timeClient.Get()) {}

const Order& LekFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& LekFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_destination == DefaultDestinations::AMEX()) {
        newOrderSingle->set(FIX::ExDestination{"AMEX"});
      } else if(info.m_fields.m_destination == DefaultDestinations::ARCA()) {
        newOrderSingle->set(FIX::ExDestination{"ARCA"});
      } else if(info.m_fields.m_destination == DefaultDestinations::BATS()) {
        newOrderSingle->set(FIX::ExDestination{"BATS"});
      } else if(info.m_fields.m_destination == DefaultDestinations::BATY()) {
        newOrderSingle->set(FIX::ExDestination{"BYX"});
      } else if(info.m_fields.m_destination == DefaultDestinations::CBSX()) {
        newOrderSingle->set(FIX::ExDestination{"CBSX"});
      } else if(info.m_fields.m_destination == DefaultDestinations::EDGA()) {
        newOrderSingle->set(FIX::ExDestination{"EDGA"});
      } else if(info.m_fields.m_destination == DefaultDestinations::EDGX()) {
        newOrderSingle->set(FIX::ExDestination{"EDGX"});
      } else if(info.m_fields.m_destination == DefaultDestinations::NASDAQ()) {
        newOrderSingle->set(FIX::ExDestination{"NSDQ"});
      } else if(info.m_fields.m_destination == DefaultDestinations::NYSE()) {
        newOrderSingle->set(FIX::ExDestination{"NYSE"});
      }
      auto& accountTag = GetAccount();
      if(accountTag.is_initialized()) {
        newOrderSingle->set(FIX::Account{*accountTag});
      }
    });
}

void LekFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::OrderCancelRequest> orderCancelRequest) {
    });
}

void LekFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport, m_timeClient->GetTime());
}

void LekFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void LekFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void LekFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void LekFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void LekFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::DoNotSend) {}

void LekFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) {}

void LekFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
    FIX::UnsupportedMessageType) {
  crack(message, sessionID);
}

void LekFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (Out<ExecutionReport> update) {
    });
}

void LekFixApplication::onMessage(const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void LekFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

const optional<string>& LekFixApplication::GetAccount() const {
  if(m_accountTag.is_initialized()) {
    return *m_accountTag;
  }
  if(GetSessionSettings().get(GetSessionId()).has("Account")) {
    m_accountTag.emplace(
      GetSessionSettings().get(GetSessionId()).getString("Account"));
  } else {
    m_accountTag.emplace(none);
  }
  return *m_accountTag;
}
