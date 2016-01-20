#include "OasisOrderExecutionServer/MatchNowFixApplication.hpp"
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/throw_exception.hpp>
#include <quickfix/Session.h>
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

namespace {
  const auto UMIR_ACCOUNT_TYPE_TAG = 47;
  const auto ANONYMOUS_TAG = 7012;
}

MatchNowFixApplication::MatchNowFixApplication(RefType<LiveNtpTimeClient>
    timeClient)
    : m_timeClient(timeClient.Get()) {}

const Order& MatchNowFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& MatchNowFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      newOrderSingle->getHeader().set(FIX::SenderSubID(GetSenderSubID()));
      newOrderSingle->set(FIX::Account(GetAccount()));
      newOrderSingle->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      auto& anonymousTag = GetAnonymousTag();
      if(anonymousTag.is_initialized()) {
        newOrderSingle->setField(ANONYMOUS_TAG, *anonymousTag);
      }
    });
}

void MatchNowFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->getHeader().set(
        FIX::SenderSubID(GetSenderSubID()));
      orderCancelRequest->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      orderCancelRequest->setField(FIX::HandlInst('1'));
    });
}

void MatchNowFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport, m_timeClient->GetTime());
}

void MatchNowFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::DoNotSend) {}

void MatchNowFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) {}

void MatchNowFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
    FIX::UnsupportedMessageType) {
  crack(message, sessionID);
}

void MatchNowFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  FIX::OrdStatus ordStatus;
  message.get(ordStatus);
  optional<OrderStatus> orderStatus = GetOrderStatus(ordStatus);
  if(orderStatus.is_initialized() &&
      *orderStatus == OrderStatus::DONE_FOR_DAY) {
    optional<OrderExecutionService::OrderId> orderId =
      FixOrderLog::GetOrderId(message);
    if(orderId.is_initialized()) {
      std::shared_ptr<PrimitiveOrder> order = m_orderLog.FindOrder(*orderId);
      if(order != nullptr) {
        order->With(
          [&] (OrderStatus status,
              const std::vector<ExecutionReport>& reports) {
            const ExecutionReport& lastReport = reports.back();
            if(lastReport.m_status == OrderStatus::PENDING_NEW) {
              ExecutionReport updatedReport =
                ExecutionReport::BuildUpdatedReport(lastReport,
                OrderStatus::NEW, m_timeClient->GetTime());
              order->Update(updatedReport);
            }
          });
      }
    }
  }
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (Out<ExecutionReport> update) {
      if(update->m_lastQuantity != 0) {
        update->m_lastMarket = DefaultMarkets::MATN().GetData();
        update->m_liquidityFlag = "A";
      }
    });
}

void MatchNowFixApplication::onMessage(
    const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void MatchNowFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

string MatchNowFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

string MatchNowFixApplication::GetSenderSubID() const {
  return GetSessionSettings().get(GetSessionId()).getString("SenderSubID");
}

const optional<string>& MatchNowFixApplication::GetAnonymousTag() const {
  if(m_anonymousTag.is_initialized()) {
    return *m_anonymousTag;
  }
  if(GetSessionSettings().get(GetSessionId()).has("Anonymous")) {
    m_anonymousTag.emplace(
      GetSessionSettings().get(GetSessionId()).getString("Anonymous"));
  } else {
    m_anonymousTag.emplace(none);
  }
  return *m_anonymousTag;
}
