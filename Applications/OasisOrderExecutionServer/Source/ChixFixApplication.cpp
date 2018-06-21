#include "OasisOrderExecutionServer/ChixFixApplication.hpp"
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
  const auto TRADE_LIQUIDITY_INDICATOR_TAG = 9882;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
}

ChixFixApplication::ChixFixApplication(RefType<LiveNtpTimeClient> timeClient)
    : m_timeClient(timeClient.Get()) {}

const Order& ChixFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& ChixFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_security.GetCountry() != DefaultCountries::CA()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid country."});
      }
      if(info.m_fields.m_currency != DefaultCurrencies::CAD()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid currency."});
      }
      newOrderSingle->getHeader().set(FIX::SenderSubID(GetSenderSubID()));
      newOrderSingle->getHeader().set(
        FIX::OnBehalfOfCompID(info.m_submissionAccount.m_name));
      newOrderSingle->set(FIX::Account(GetAccount()));
      newOrderSingle->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      newOrderSingle->setField(UMIR_USER_ID_TAG, GetUmirUserID());
      newOrderSingle->setField(BROKER_NUMBER_TAG, GetBrokerNumber());
      auto& anonymousTag = GetAnonymousTag();
      if(anonymousTag.is_initialized()) {
        newOrderSingle->setField(ANONYMOUS_TAG, *anonymousTag);
      }
      bool hasDestination = false;
      for(auto& tag : info.m_fields.m_additionalFields) {
        if(tag.GetKey() == FIX::FIELD::ExDestination) {
          if(auto value = boost::get<std::string>(&tag.GetValue())) {
            FIX::ExDestination destination(*value);
            newOrderSingle->getHeader().setField(destination);
            if(*value == "SMRTXDARKNR") {
              newOrderSingle->setField(ANONYMOUS_TAG, "Y");
            }
            hasDestination = true;
            break;
          } else {
            BOOST_THROW_EXCEPTION(FixOrderRejectedException(
              "Invalid value for tag 100 (ExDestination)."));
          }
        }
      }
      auto noTradeFeat = GetNoTradeFeat();
      auto noTradeKey = GetNoTradeKey();
      if(!noTradeFeat.empty() && !noTradeKey.empty()) {
        newOrderSingle->setField(NO_TRADE_FEAT_TAG, noTradeFeat);
        newOrderSingle->setField(NO_TRADE_KEY_TAG, noTradeKey);
      }
      if(!hasDestination) {
        if(info.m_fields.m_type == OrderType::PEGGED) {
          FIX::TargetSubID route;
          if(info.m_fields.m_destination == DefaultDestinations::CHIX()) {
            route = FIX::TargetSubID("CHIX");
          } else if(info.m_fields.m_destination ==
              DefaultDestinations::CX2()) {
            route = FIX::TargetSubID("CX2");
          } else {
            BOOST_THROW_EXCEPTION(
              FixOrderRejectedException("Destination not supported."));
          }
          newOrderSingle->getHeader().setField(route);
        } else {
          FIX::TargetSubID route;
          if(info.m_fields.m_destination == DefaultDestinations::CHIX()) {
            route = FIX::TargetSubID("SMRTCHIX");
          } else if(info.m_fields.m_destination ==
              DefaultDestinations::CX2()) {
            route = FIX::TargetSubID("SMRTCX2");
          } else {
            BOOST_THROW_EXCEPTION(
              FixOrderRejectedException("Destination not supported."));
          }
          newOrderSingle->getHeader().setField(route);
        }
      }
    });
}

void ChixFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->getHeader().set(
        FIX::SenderSubID(GetSenderSubID()));
      orderCancelRequest->getHeader().set(
        FIX::OnBehalfOfCompID(session.GetAccount().m_name));
      orderCancelRequest->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      orderCancelRequest->setField(UMIR_USER_ID_TAG, GetUmirUserID());
      orderCancelRequest->setField(BROKER_NUMBER_TAG, GetBrokerNumber());
    });
}

void ChixFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport, m_timeClient->GetTime());
}

void ChixFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void ChixFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void ChixFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void ChixFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void ChixFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::DoNotSend) {}

void ChixFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) {}

void ChixFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
    FIX::UnsupportedMessageType) {
  crack(message, sessionID);
}

void ChixFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      std::string liquidityFlag;
      if(message.isSetField(TRADE_LIQUIDITY_INDICATOR_TAG)) {
        liquidityFlag = message.getField(TRADE_LIQUIDITY_INDICATOR_TAG);
      }
      if(liquidityFlag == "A") {
        update->m_liquidityFlag = "P";
      } else if(liquidityFlag == "R") {
        update->m_liquidityFlag = "A";
      }
      FIX::ExecBroker execBroker;
      if(message.isSet(execBroker)) {
        message.get(execBroker);
      }
      if(execBroker == "AEQN") {
        update->m_lastMarket = DefaultMarkets::NEOE().GetData();
      } else if(execBroker == "AEQL") {
        update->m_lastMarket = DefaultMarkets::NEOE().GetData();
      } else if(execBroker == "CHIX") {
        update->m_lastMarket = DefaultMarkets::CHIC().GetData();
      } else if(execBroker == "CX2") {
        update->m_lastMarket = DefaultMarkets::XCX2().GetData();
      } else if(execBroker == "TSX") {
        update->m_lastMarket = DefaultMarkets::TSX().GetData();
      } else if(execBroker == "PURE") {
        if(order.GetInfo().m_fields.m_security.GetMarket() ==
            DefaultMarkets::CSE()) {
          update->m_lastMarket = DefaultMarkets::CSE().GetData();
        } else {
          update->m_lastMarket = DefaultMarkets::PURE().GetData();
        }
      } else if(execBroker == "ALPH") {
        update->m_lastMarket = DefaultMarkets::XATS().GetData();
      } else if(execBroker == "LYNX") {
        update->m_lastMarket = DefaultMarkets::LYNX().GetData();
      } else if(execBroker == "MATCH") {
        update->m_lastMarket = DefaultMarkets::MATN().GetData();
      } else if(execBroker == "OMGA") {
        update->m_lastMarket = DefaultMarkets::OMGA().GetData();
      }
    });
}

void ChixFixApplication::onMessage(const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void ChixFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

std::string ChixFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

std::string ChixFixApplication::GetSenderSubID() const {
  return GetSessionSettings().get(GetSessionId()).getString("SenderSubID");
}

std::string ChixFixApplication::GetUmirUserID() const {
  if(GetSessionSettings().get(GetSessionId()).has("UMIRUserID")) {
    return GetSessionSettings().get(GetSessionId()).getString("UMIRUserID");
  }
  return GetSenderSubID();
}

std::string ChixFixApplication::GetBrokerNumber() const {
  if(GetSessionSettings().get(GetSessionId()).has("BrokerNumber")) {
    return GetSessionSettings().get(GetSessionId()).getString("BrokerNumber");
  }
  return GetSenderSubID();
}

std::string ChixFixApplication::GetNoTradeFeat() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeFeat")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeFeat");
  }
  return {};
}

std::string ChixFixApplication::GetNoTradeKey() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeKey")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeKey");
  }
  return {};
}

const optional<std::string>& ChixFixApplication::GetAnonymousTag() const {
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
