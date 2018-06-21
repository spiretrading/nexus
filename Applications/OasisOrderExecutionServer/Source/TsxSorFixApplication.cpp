#include "OasisOrderExecutionServer/TsxSorFixApplication.hpp"
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
  const auto TSX_ACCOUNT_TYPE_TAG = 6750;
  const auto TSX_USER_ID_TAG = 6751;
  const auto TSX_ANONYMOUS_TAG = 6761;
  const auto TSX_EXCHANGE_ADMIN_TAG = 6780;
  const auto TSX_NO_TRADE_FEAT_TAG = 7713;
  const auto TSX_NO_TRADE_KEY_TAG = 7714;
  const auto TSX_PEG_TYPE_TAG = 7723;
  const auto TSX_UNDISPLAYED_TAG = 7726;
}

TsxSorFixApplication::TsxSorFixApplication(RefType<LiveNtpTimeClient>
    timeClient)
    : m_timeClient(timeClient.Get()) {}

const Order& TsxSorFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& TsxSorFixApplication::Submit(const OrderInfo& info) {
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
      newOrderSingle->set(FIX::HandlInst('6'));
      auto& anonymousTag = GetAnonymousTag();
      if(anonymousTag.is_initialized()) {
        newOrderSingle->setField(TSX_ANONYMOUS_TAG, *anonymousTag);
      }
      if(info.m_fields.m_type == OrderType::PEGGED) {
        if(info.m_fields.m_destination == DefaultDestinations::TSX()) {
          newOrderSingle->setField(TSX_PEG_TYPE_TAG, "M");
          newOrderSingle->setField(TSX_UNDISPLAYED_TAG, "Y");
        } else {
          BOOST_THROW_EXCEPTION(
            FixOrderRejectedException("Pegged order type not supported."));
        }
      }
      if(info.m_fields.m_timeInForce.GetType() == TimeInForce::Type::MOC) {
        optional<FIX::TimeInForce> timeInForce =
          GetTimeInForceType(TimeInForce::Type::DAY);
        if(timeInForce.is_initialized()) {
          newOrderSingle->set(*timeInForce);
          if(info.m_fields.m_type == OrderType::MARKET) {
            newOrderSingle->set(FIX::OrdType(FIX::OrdType_MARKET_ON_CLOSE));
          } else if(info.m_fields.m_type == OrderType::LIMIT) {
            newOrderSingle->set(FIX::OrdType(FIX::OrdType_LIMIT_ON_CLOSE));
          }
        }
      }
      auto noTradeFeat = GetNoTradeFeat();
      auto noTradeKey = GetNoTradeKey();
      if(!noTradeFeat.empty() && !noTradeKey.empty()) {
        newOrderSingle->setField(TSX_NO_TRADE_FEAT_TAG, noTradeFeat);
        newOrderSingle->setField(TSX_NO_TRADE_KEY_TAG, noTradeKey);
      }
      newOrderSingle->setField(TSX_ACCOUNT_TYPE_TAG, "CL");
      newOrderSingle->setField(TSX_USER_ID_TAG, GetTsxUserID());
      std::string adminField;
      if(info.m_fields.m_destination == DefaultDestinations::ALPHA()) {
        adminField = "00000A00";
      }
      if(!adminField.empty()) {
        newOrderSingle->setField(TSX_EXCHANGE_ADMIN_TAG, adminField);
      }
    });
}

void TsxSorFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->getHeader().set(
        FIX::SenderSubID(GetSenderSubID()));
      orderCancelRequest->getHeader().set(
        FIX::OnBehalfOfCompID(session.GetAccount().m_name));
      orderCancelRequest->setField(TSX_ACCOUNT_TYPE_TAG, "CL");
      orderCancelRequest->setField(TSX_USER_ID_TAG, GetTsxUserID());
    });
}

void TsxSorFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport,
    m_timeClient->GetTime());
}

void TsxSorFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void TsxSorFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void TsxSorFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void TsxSorFixApplication::toAdmin(FIX::Message& message,
  const FIX::SessionID& sessionID) {}

void TsxSorFixApplication::toApp(FIX::Message& message,
  const FIX::SessionID& sessionID) throw(FIX::DoNotSend) {}

void TsxSorFixApplication::fromAdmin(const FIX::Message& message,
  const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
  FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) {}

void TsxSorFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
    FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
    FIX::UnsupportedMessageType) {
  crack(message, sessionID);
}

void TsxSorFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  FIX::ExecRestatementReason restatementReason;
  if(message.isSet(restatementReason)) {
    return;
  }
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      std::string tsxExchangeAdminValue;
      if(message.isSetField(TSX_EXCHANGE_ADMIN_TAG)) {
        tsxExchangeAdminValue = message.getField(TSX_EXCHANGE_ADMIN_TAG);
      }
      if(tsxExchangeAdminValue.size() >= 7) {
        char liquidityFlag = tsxExchangeAdminValue[1];
        if(liquidityFlag != '0') {
          update->m_liquidityFlag = tsxExchangeAdminValue[1];
          char session = tsxExchangeAdminValue[2];
          if(session == 'O' || session == 'E') {
            update->m_liquidityFlag += session;
          }
          char lastMarket = tsxExchangeAdminValue[6];
          if(lastMarket == 'X') {
            update->m_lastMarket = DefaultMarkets::TSX().GetData();
          } else if(lastMarket == 'V') {
            update->m_lastMarket = DefaultMarkets::TSXV().GetData();
          } else if(lastMarket == 'P') {
            update->m_lastMarket = DefaultMarkets::PURE().GetData();
          } else if(lastMarket == 'C') {
            update->m_lastMarket = DefaultMarkets::CHIC().GetData();
          } else if(lastMarket == 'O') {
            update->m_lastMarket = DefaultMarkets::OMGA().GetData();
          } else if(lastMarket == 'A') {
            update->m_lastMarket = DefaultMarkets::XATS().GetData();
          } else if(lastMarket == '2') {
            update->m_lastMarket = DefaultMarkets::XCX2().GetData();
          } else if(lastMarket == 'Y') {
            update->m_lastMarket = DefaultMarkets::LYNX().GetData();
          }
        }
      }
    });
}

void TsxSorFixApplication::onMessage(
    const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void TsxSorFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

std::string TsxSorFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

std::string TsxSorFixApplication::GetSenderSubID() const {
  return GetSessionSettings().get(GetSessionId()).getString("SenderSubID");
}

std::string TsxSorFixApplication::GetTsxUserID() const {
  if(GetSessionSettings().get(GetSessionId()).has("TSXUserID")) {
    return GetSessionSettings().get(GetSessionId()).getString("TSXUserID");
  }
  return GetSenderSubID();
}

std::string TsxSorFixApplication::GetNoTradeFeat() const {
  if(GetSessionSettings().get(GetSessionId()).has("TSXNoTradeFeat")) {
    return GetSessionSettings().get(GetSessionId()).getString(
      "TSXNoTradeFeat");
  }
  return {};
}

std::string TsxSorFixApplication::GetNoTradeKey() const {
  if(GetSessionSettings().get(GetSessionId()).has("TSXNoTradeKey")) {
    return GetSessionSettings().get(GetSessionId()).getString("TSXNoTradeKey");
  }
  return {};
}

const optional<std::string>& TsxSorFixApplication::GetAnonymousTag() const {
  if(m_anonymousTag.is_initialized()) {
    return *m_anonymousTag;
  }
  if(GetSessionSettings().get(GetSessionId()).has("TSXAnonymous")) {
    m_anonymousTag.emplace(
      GetSessionSettings().get(GetSessionId()).getString("TSXAnonymous"));
  } else {
    m_anonymousTag.emplace(none);
  }
  return *m_anonymousTag;
}
