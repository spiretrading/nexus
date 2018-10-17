#include "OasisOrderExecutionServer/AequitasFixApplication.hpp"
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

namespace {
  const auto AGGRESSOR_INDICATOR_TAG = 1057;
  const auto UMIR_ACCOUNT_TYPE_TAG = 6750;
  const auto UMIR_USER_ID_TAG = 6751;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
  const auto VISIBILITY_TYPE_TAG = 20000;
}

AequitasFixApplication::AequitasFixApplication(
    RefType<LiveNtpTimeClient> timeClient)
    : m_timeClient(timeClient.Get()) {}

const Order& AequitasFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& AequitasFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_security.GetCountry() != DefaultCountries::CA()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid country."});
      }
      if(info.m_fields.m_currency != DefaultCurrencies::CAD()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid currency."});
      }
      newOrderSingle->getHeader().set(
        FIX::OnBehalfOfCompID(info.m_submissionAccount.m_name));
      newOrderSingle->set(FIX::Account(GetAccount()));
      newOrderSingle->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      newOrderSingle->setField(UMIR_USER_ID_TAG, GetUmirUserID());
      auto isProtected = true;
      auto isMidPoint = false;
      auto isNeoBook = false;
      for(auto& tag : info.m_fields.m_additionalFields) {
        if(tag.GetKey() == FIX::FIELD::ExecInst) {
          if(auto value = boost::get<string>(&tag.GetValue())) {
            if(*value == "M") {
              isProtected = false;
              isMidPoint = true;
            }
          } else {
            BOOST_THROW_EXCEPTION(FixOrderRejectedException(
              "Invalid value for tag 18 (ExecInst)."));
          }
        } else if(tag.GetKey() == FIX::FIELD::ExDestination) {
          if(auto value = boost::get<string>(&tag.GetValue())) {
            FIX::ExDestination destination(*value);
            newOrderSingle->getHeader().setField(destination);
            if(*value == "N") {
              isNeoBook = true;
            }
            break;
          } else {
            BOOST_THROW_EXCEPTION(FixOrderRejectedException(
              "Invalid value for tag 100 (ExDestination)."));
          }
        }
      }
      if(isProtected) {
        newOrderSingle->set(FIX::HandlInst('5'));
      }
      if(isMidPoint) {
        if(!isNeoBook) {
          newOrderSingle->setField(VISIBILITY_TYPE_TAG, "2");
        }
      }
      auto noTradeFeat = GetNoTradeFeat();
      auto noTradeKey = GetNoTradeKey();
      if(!noTradeFeat.empty() && !noTradeKey.empty()) {
        newOrderSingle->setField(NO_TRADE_FEAT_TAG, noTradeFeat);
        newOrderSingle->setField(NO_TRADE_KEY_TAG, noTradeKey);
      }
    });
}

void AequitasFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->getHeader().set(
        FIX::OnBehalfOfCompID(session.GetAccount().m_name));
      orderCancelRequest->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      orderCancelRequest->setField(UMIR_USER_ID_TAG, GetUmirUserID());
    });
}

void AequitasFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport,
    m_timeClient->GetTime());
}

void AequitasFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void AequitasFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void AequitasFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void AequitasFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void AequitasFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void AequitasFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void AequitasFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) {
  crack(message, sessionID);
}

void AequitasFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      string liquidityFlag;
      if(message.isSetField(AGGRESSOR_INDICATOR_TAG)) {
        liquidityFlag = message.getField(AGGRESSOR_INDICATOR_TAG);
      }
      if(liquidityFlag == "Y") {
        update->m_liquidityFlag = "A";
      } else if(liquidityFlag == "N") {
        update->m_liquidityFlag = "P";
      }
      if(!liquidityFlag.empty()) {
        update->m_lastMarket = DefaultMarkets::NEOE().GetData();
      }
    });
}

void AequitasFixApplication::onMessage(
    const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void AequitasFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

string AequitasFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

string AequitasFixApplication::GetUmirUserID() const {
  if(GetSessionSettings().get(GetSessionId()).has("UMIRUserID")) {
    return GetSessionSettings().get(GetSessionId()).getString("UMIRUserID");
  }
  return {};
}

string AequitasFixApplication::GetNoTradeFeat() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeFeat")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeFeat");
  }
  return {};
}

string AequitasFixApplication::GetNoTradeKey() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeKey")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeKey");
  }
  return {};
}
