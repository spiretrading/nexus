#include "OasisOrderExecutionServer/CnsxFixApplication.hpp"
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

namespace {
  const auto UMIR_ACCOUNT_TYPE_TAG = 6750;
  const auto UMIR_USER_ID_TAG = 6751;
  const auto ANONYMOUS_TAG = 6761;
  const auto BROKER_NUMBER_TAG = 6774;
  const auto EXCHANGE_ADMIN_TAG = 6780;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
  const auto TRADE_LIQUIDITY_INDICATOR_TAG = 9882;
}

CnsxFixApplication::CnsxFixApplication(Ref<LiveNtpTimeClient> timeClient)
  : m_timeClient(timeClient.Get()) {}

const Order& CnsxFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& CnsxFixApplication::Submit(const OrderInfo& info) {
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
      newOrderSingle->getHeader().set(
        FIX::OnBehalfOfCompID(info.m_submissionAccount.m_name));
      newOrderSingle->set(FIX::Account(GetAccount()));
      newOrderSingle->set(FIX::HandlInst('6'));
      newOrderSingle->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      newOrderSingle->setField(UMIR_USER_ID_TAG, GetUmirUserID());
      auto& anonymousTag = GetAnonymousTag();
      if(anonymousTag) {
        newOrderSingle->setField(ANONYMOUS_TAG, *anonymousTag);
      }
      auto noTradeFeat = GetNoTradeFeat();
      if(!noTradeFeat.empty()) {
        newOrderSingle->setField(NO_TRADE_FEAT_TAG, noTradeFeat);
      }
      auto noTradeKey = GetNoTradeKey();
      if(!noTradeKey.empty()) {
        newOrderSingle->setField(NO_TRADE_KEY_TAG, noTradeKey);
      }
      if(m_lei) {
        m_lei->populate(Store(newOrderSingle));
      }
    });
}

void CnsxFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->getHeader().set(FIX::SenderSubID(GetSenderSubID()));
      orderCancelRequest->getHeader().set(
        FIX::OnBehalfOfCompID(session.GetAccount().m_name));
      orderCancelRequest->setField(UMIR_USER_ID_TAG, GetUmirUserID());
    });
}

void CnsxFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(
    session, orderId, executionReport, m_timeClient->GetTime());
}

void CnsxFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void CnsxFixApplication::onLogon(const FIX::SessionID& sessionID) {
  m_lei.emplace(GetSessionSettings().get(GetSessionId()));
}

void CnsxFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void CnsxFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void CnsxFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void CnsxFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void CnsxFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) {
  crack(message, sessionID);
}

void CnsxFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      if(order.GetInfo().m_fields.m_destination ==
          DefaultDestinations::CSE2()) {
        auto flag = [&] () -> std::string {
          if(message.isSetField(TRADE_LIQUIDITY_INDICATOR_TAG)) {
            return message.getField(TRADE_LIQUIDITY_INDICATOR_TAG);
          }
          return {};
        }();
        update->m_liquidityFlag = std::move(flag);
        update->m_lastMarket = DefaultMarkets::CSE2().GetData();
      } else {
        auto exchangeAdminValue = [&] () -> std::string {
          if(message.isSetField(EXCHANGE_ADMIN_TAG)) {
            return message.getField(EXCHANGE_ADMIN_TAG);
          }
          return {};
        }();
        if(exchangeAdminValue.size() >= 2) {
          auto liquidityFlag = exchangeAdminValue[1];
          if(liquidityFlag != '0' && exchangeAdminValue.size() >= 3) {
            update->m_liquidityFlag = exchangeAdminValue[1];
            auto session = exchangeAdminValue[2];
            if(session == 'O' || session == 'E') {
              update->m_liquidityFlag += session;
            }
          }
          auto lastMarket = exchangeAdminValue[0];
          if(lastMarket == 'P') {
            update->m_lastMarket = DefaultMarkets::PURE().GetData();
          } else if(lastMarket == 'Q') {
            update->m_lastMarket = DefaultMarkets::CSE().GetData();
          }
        }
      }
    });
}

void CnsxFixApplication::onMessage(const FIX42::TradingSessionStatus& message,
  const FIX::SessionID& sessionId) {}

void CnsxFixApplication::onMessage(const FIX42::OrderCancelReject& message,
  const FIX::SessionID& sessionId) {}

std::string CnsxFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

std::string CnsxFixApplication::GetSenderSubID() const {
  return GetSessionSettings().get(GetSessionId()).getString("SenderSubID");
}

std::string CnsxFixApplication::GetUmirUserID() const {
  if(GetSessionSettings().get(GetSessionId()).has("UMIRUserID")) {
    return GetSessionSettings().get(GetSessionId()).getString("UMIRUserID");
  }
  return GetSenderSubID();
}

std::string CnsxFixApplication::GetNoTradeFeat() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeFeat")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeFeat");
  }
  return "";
}

std::string CnsxFixApplication::GetNoTradeKey() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeKey")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeKey");
  }
  return "";
}

const optional<std::string>& CnsxFixApplication::GetAnonymousTag() const {
  if(m_anonymousTag) {
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
