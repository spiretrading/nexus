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
using namespace Nexus::MarketDataService;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Queries;

MonexBoomFixApplication::MonexBoomFixApplication(
  Ref<LiveNtpTimeClient> timeClient,
  Ref<ApplicationMarketDataClient::Client> marketDataClient)
  : m_timeClient(timeClient.Get()),
    m_marketDataClient(marketDataClient.Get()) {}

const Order& MonexBoomFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& MonexBoomFixApplication::Submit(const OrderInfo& info) {
  auto modifiedInfo = info;
  if(modifiedInfo.m_fields.m_type == OrderType::MARKET) {
    modifiedInfo.m_fields.m_type = OrderType::LIMIT;
    auto bboQuote = LoadBboQuote(modifiedInfo.m_fields.m_security);
    if(modifiedInfo.m_fields.m_side == Side::BID) {
      modifiedInfo.m_fields.m_price = bboQuote.m_ask.m_price;
    } else {
      modifiedInfo.m_fields.m_price = bboQuote.m_bid.m_price;
    }
  }
  modifiedInfo.m_shortingFlag = false;
  return m_orderLog.Submit(modifiedInfo, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX44::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_destination == DefaultDestinations::HKEX()) {
        if(info.m_fields.m_security.GetCountry() != DefaultCountries::HK()) {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid country."));
        }
        if(info.m_fields.m_currency != DefaultCurrencies::HKD()) {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid currency."));
        }
      } else if(info.m_fields.m_destination == DefaultDestinations::TSE() ||
          info.m_fields.m_destination == DefaultDestinations::OSE()) {
        if(info.m_fields.m_security.GetCountry() != DefaultCountries::JP()) {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid country."));
        }
        if(info.m_fields.m_currency != DefaultCurrencies::JPY()) {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid currency."));
        }
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException(
          "Invalid destination."));
      }
      newOrderSingle->set(FIX::SecurityExchange(
        info.m_fields.m_security.GetMarket().GetData()));
      newOrderSingle->set(GetAccount());
    });
}

void MonexBoomFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX44::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->set(FIX::SecurityExchange(
        order.GetInfo().m_fields.m_security.GetMarket().GetData()));
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
        update->m_liquidityFlag = lexical_cast<std::string>(
          LiquidityFlag::ACTIVE);
        update->m_lastMarket = order.GetInfo().m_fields.m_destination;
      }
    });
}

void MonexBoomFixApplication::onMessage(
  const FIX44::TradingSessionStatus& message,
  const FIX::SessionID& sessionId) {}

void MonexBoomFixApplication::onMessage(const FIX44::OrderCancelReject& message,
  const FIX::SessionID& sessionId) {}

BboQuote MonexBoomFixApplication::LoadBboQuote(const Security& security) {
  auto bbo = m_bboQuotes.GetOrInsert(security,
    [&] {
      auto bbo = std::make_shared<StateQueue<BboQuote>>();
      QueryRealTimeWithSnapshot(security, *m_marketDataClient, bbo);
      return bbo;
    });
  try {
    return bbo->Peek();
  } catch(const Beam::PipeBrokenException&) {
    m_bboQuotes.Erase(security);
    BOOST_THROW_EXCEPTION(
      FixOrderRejectedException{"No BBO quote available."});
  }
}

FIX::Account MonexBoomFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

FIX::Username MonexBoomFixApplication::GetUsername() const {
  return GetSessionSettings().get(GetSessionId()).getString("Username");
}

FIX::Password MonexBoomFixApplication::GetPassword() const {
  return GetSessionSettings().get(GetSessionId()).getString("Password");
}
