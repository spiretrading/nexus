#include "OasisOrderExecutionServer/SerenityFixApplication.hpp"
#include <quickfix/Session.h>

using namespace Beam;
using namespace Beam::TimeService;
using namespace Nexus;
using namespace Nexus::FixUtilities;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;

SerenityFixApplication::SerenityFixApplication(
  Ref<LiveNtpTimeClient> timeClient)
  : m_timeClient(timeClient.Get()) {}

const Order& SerenityFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& SerenityFixApplication::Submit(const OrderInfo& info) {
  throw std::runtime_error("Not implemented.");
}

void SerenityFixApplication::Cancel(
    const OrderExecutionSession& session, OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
      Out<FIX42::OrderCancelRequest> orderCancelRequest) {});
}

void SerenityFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport, m_timeClient->GetTime());
}

void SerenityFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void SerenityFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void SerenityFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void SerenityFixApplication::toAdmin(
  FIX::Message& message, const FIX::SessionID& sessionID) {}

void SerenityFixApplication::toApp(
  FIX::Message& message, const FIX::SessionID& sessionID) {}

void SerenityFixApplication::fromAdmin(
  const FIX::Message& message, const FIX::SessionID& sessionID) {}

void SerenityFixApplication::fromApp(
    const FIX::Message& message, const FIX::SessionID& sessionID) {
  crack(message, sessionID);
}

void SerenityFixApplication::onMessage(
    const FIX42::ExecutionReport& message, const FIX::SessionID& sessionId) {}

void SerenityFixApplication::onMessage(
  const FIX42::TradingSessionStatus& message,
  const FIX::SessionID& sessionId) {}

void SerenityFixApplication::onMessage(
  const FIX42::OrderCancelReject& message, const FIX::SessionID& sessionId) {}
