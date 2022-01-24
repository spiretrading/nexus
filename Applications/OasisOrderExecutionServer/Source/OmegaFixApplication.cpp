#include "OasisOrderExecutionServer/OmegaFixApplication.hpp"
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
using namespace Nexus::MarketDataService;
using namespace Nexus::Queries;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto UMIR_ACCOUNT_TYPE_TAG = 6750;
  const auto UMIR_USER_ID_TAG = 6751;
  const auto ANONYMOUS_TAG = 6761;
  const auto POST_ON_MARKET_TAG = 6888;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
  const auto TRADE_LIQUIDITY_INDICATOR_TAG = 9730;
}

OmegaFixApplication::OmegaFixApplication(Ref<LiveNtpTimeClient> timeClient,
    Ref<ApplicationMarketDataClient::Client> marketDataClient)
    : m_timeClient(timeClient.Get()),
      m_marketDataClient(marketDataClient.Get()) {}

const Order& OmegaFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& OmegaFixApplication::Submit(const OrderInfo& info) {
  auto modifiedInfo = std::optional<OrderInfo>();
  const OrderInfo* submissionInfo;
  if(info.m_fields.m_type == OrderType::MARKET) {
    modifiedInfo.emplace(info);
    submissionInfo = &*modifiedInfo;
    modifiedInfo->m_fields.m_type = OrderType::LIMIT;
    auto bboQuote = LoadBboQuote(modifiedInfo->m_fields.m_security);
    if(info.m_fields.m_side == Side::BID) {
      modifiedInfo->m_fields.m_price = bboQuote.m_ask.m_price +
        2 * Money::CENT;
    } else {
      modifiedInfo->m_fields.m_price = std::max(
        bboQuote.m_bid.m_price - 2 * Money::CENT, Money::CENT / 2);
    }
  } else {
    submissionInfo = &info;
  }
  return m_orderLog.Submit(*submissionInfo, GetSessionId().getSenderCompID(),
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
      newOrderSingle->set(FIX::ExecBroker(GetExecBroker()));
      newOrderSingle->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      newOrderSingle->setField(UMIR_USER_ID_TAG, GetUmirUserID());
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
      if(submissionInfo->m_fields.m_destination ==
          DefaultDestinations::OMEGA()) {
        FIX::ExDestination exDestination(GetOmegaRoute());
        newOrderSingle->setField(exDestination);
        newOrderSingle->setField(POST_ON_MARKET_TAG, "OMGA");
      } else if(submissionInfo->m_fields.m_destination ==
          DefaultDestinations::LYNX()) {
        FIX::ExDestination exDestination(GetLynxRoute());
        newOrderSingle->setField(exDestination);
        newOrderSingle->setField(POST_ON_MARKET_TAG, "LYNX");
      }
    });
}

void OmegaFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->getHeader().set(
        FIX::OnBehalfOfCompID(session.GetAccount().m_name));
      orderCancelRequest->setField(UMIR_USER_ID_TAG, GetUmirUserID());
    });
}

void OmegaFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport,
    m_timeClient->GetTime());
}

void OmegaFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void OmegaFixApplication::onLogon(const FIX::SessionID& sessionID) {
  m_lei.emplace(GetSessionSettings().get(GetSessionId()));
}

void OmegaFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void OmegaFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void OmegaFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void OmegaFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void OmegaFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) {
  crack(message, sessionID);
}

void OmegaFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      if(update->m_lastQuantity != 0) {
        std::string liquidityFlag;
        if(message.isSetField(TRADE_LIQUIDITY_INDICATOR_TAG)) {
          liquidityFlag = message.getField(TRADE_LIQUIDITY_INDICATOR_TAG);
        }
        if(liquidityFlag == "A") {
          update->m_liquidityFlag = "P";
        } else if(liquidityFlag == "R") {
          update->m_liquidityFlag = "A";
        }
      }
      FIX::LastMkt lastMkt;
      if(message.isSet(lastMkt)) {
        message.get(lastMkt);
      }
      if(lastMkt == "XATS") {
        update->m_lastMarket = DefaultMarkets::XATS().GetData();
      } else if(lastMkt == "CHIC") {
        update->m_lastMarket = DefaultMarkets::CHIC().GetData();
      } else if(lastMkt == "XCX2") {
        update->m_lastMarket = DefaultMarkets::XCX2().GetData();
      } else if(lastMkt == "LYNX") {
        update->m_lastMarket = DefaultMarkets::LYNX().GetData();
      } else if(lastMkt == "OMGA") {
        update->m_lastMarket = DefaultMarkets::OMGA().GetData();
      } else if(lastMkt == "PURE") {
        if(order.GetInfo().m_fields.m_security.GetMarket() ==
            DefaultMarkets::CSE()) {
          update->m_lastMarket = DefaultMarkets::CSE().GetData();
        } else {
          update->m_lastMarket = DefaultMarkets::PURE().GetData();
        }
      } else if(lastMkt == "XTSE") {
        if(order.GetInfo().m_fields.m_security.GetMarket() ==
            DefaultMarkets::TSXV()) {
          update->m_lastMarket = DefaultMarkets::TSXV().GetData();
        } else {
          update->m_lastMarket = DefaultMarkets::TSX().GetData();
        }
      } else if(lastMkt == "XTSX") {
        update->m_lastMarket = DefaultMarkets::TSXV().GetData();
      }
    });
}

void OmegaFixApplication::onMessage(const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void OmegaFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

BboQuote OmegaFixApplication::LoadBboQuote(const Security& security) {
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

std::string OmegaFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

std::string OmegaFixApplication::GetExecBroker() const {
  return GetSessionSettings().get(GetSessionId()).getString("ExecBroker");
}

std::string OmegaFixApplication::GetUmirUserID() const {
  return GetSessionSettings().get(GetSessionId()).getString("UMIRUserID");
}

std::string OmegaFixApplication::GetOmegaRoute() const {
  return GetSessionSettings().get(GetSessionId()).getString("OmegaRoute");
}

std::string OmegaFixApplication::GetLynxRoute() const {
  return GetSessionSettings().get(GetSessionId()).getString("LynxRoute");
}

const optional<std::string>& OmegaFixApplication::GetAnonymousTag() const {
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

std::string OmegaFixApplication::GetNoTradeFeat() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeFeat")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeFeat");
  }
  return {};
}

std::string OmegaFixApplication::GetNoTradeKey() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeKey")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeKey");
  }
  return {};
}
