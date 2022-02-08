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

namespace {
  const auto UMIR_ACCOUNT_TYPE_TAG = 47;
  const auto ANONYMOUS_TAG = 7012;
  const auto CONSTRAINTS_TAG = 6005;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
}

MatchNowFixApplication::MatchNowFixApplication(
  Ref<LiveNtpTimeClient> timeClient)
  : m_timeClient(timeClient.Get()) {}

const Order& MatchNowFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& MatchNowFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_security.GetCountry() != DefaultCountries::CA()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid country."));
      }
      if(info.m_fields.m_currency != DefaultCurrencies::CAD()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid currency."));
      }
      newOrderSingle->getHeader().set(FIX::SenderSubID(GetSenderSubID()));
      newOrderSingle->set(FIX::Account(GetAccount()));
      newOrderSingle->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      auto& anonymousTag = GetAnonymousTag();
      if(anonymousTag.is_initialized()) {
        newOrderSingle->setField(ANONYMOUS_TAG, *anonymousTag);
      }
      auto noTradeFeat = GetNoTradeFeat();
      auto noTradeKey = GetNoTradeKey();
      if(!noTradeFeat.empty() && !noTradeKey.empty()) {
        newOrderSingle->setField(NO_TRADE_FEAT_TAG, noTradeFeat);
        newOrderSingle->setField(NO_TRADE_KEY_TAG, noTradeKey);
      }
      if(m_lei) {
        m_lei->populate(Store(newOrderSingle));
      }
      auto constraintsTagIterator = std::find_if(
        info.m_fields.m_additionalFields.begin(),
        info.m_fields.m_additionalFields.end(),
        [] (const Tag& tag) {
          return tag.GetKey() == CONSTRAINTS_TAG;
        });
      if(constraintsTagIterator != info.m_fields.m_additionalFields.end()) {
        auto& constraintsTag = *constraintsTagIterator;
        auto value = boost::get<std::string>(constraintsTag.GetValue());
        if(value == "PAG") {
          newOrderSingle->setField(CONSTRAINTS_TAG, "PAG=-1");
          if(info.m_fields.m_destination == "MATNLP") {
            newOrderSingle->setField(FIX::ExecInst("R"));
          }
        } else if(value == "PMI") {
          newOrderSingle->setField(CONSTRAINTS_TAG, "PMI=1");
          if(info.m_fields.m_destination == "MATNLP") {
            newOrderSingle->setField(FIX::ExecInst("p"));
          }
        }
      } else if(info.m_fields.m_destination == "MATNLP") {
        newOrderSingle->setField(FIX::ExecInst("M"));
      }
    });
}

void MatchNowFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->getHeader().set(
        FIX::SenderSubID(GetSenderSubID()));
      orderCancelRequest->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      orderCancelRequest->setField(FIX::HandlInst('1'));
    });
}

void MatchNowFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport,
    m_timeClient->GetTime());
}

void MatchNowFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::onLogon(const FIX::SessionID& sessionID) {
  m_lei.emplace(GetSessionSettings().get(GetSessionId()));
}

void MatchNowFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void MatchNowFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) {
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
                ExecutionReport::MakeUpdatedReport(lastReport,
                  OrderStatus::NEW, m_timeClient->GetTime());
              order->Update(updatedReport);
            }
          });
      }
    }
  }
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
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

std::string MatchNowFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

std::string MatchNowFixApplication::GetSenderSubID() const {
  return GetSessionSettings().get(GetSessionId()).getString("SenderSubID");
}

const optional<std::string>& MatchNowFixApplication::GetAnonymousTag() const {
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

std::string MatchNowFixApplication::GetNoTradeFeat() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeFeat")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeFeat");
  }
  return {};
}

std::string MatchNowFixApplication::GetNoTradeKey() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeKey")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeKey");
  }
  return {};
}
