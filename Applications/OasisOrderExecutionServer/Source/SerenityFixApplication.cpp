#include "OasisOrderExecutionServer/SerenityFixApplication.hpp"
#include <quickfix/Session.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"

using namespace boost;
using namespace Beam;
using namespace Beam::TimeService;
using namespace Nexus;
using namespace Nexus::FixUtilities;
using namespace Nexus::MarketDataService;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto MATN_CONSTRAINTS_TAG = 6005;
  const auto UMIR_ACCOUNT_TYPE_TAG = 6750;
  const auto UMIR_USER_ID_TAG = 6751;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
  const auto LONG_LIFE_TAG = 7735;
}

SerenityFixApplication::SerenityFixApplication(
  Ref<LiveNtpTimeClient> timeClient,
  Ref<ApplicationMarketDataClient::Client> marketDataClient)
  : m_timeClient(timeClient.Get()),
    m_marketDataClient(marketDataClient.Get()) {}

const Order& SerenityFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& SerenityFixApplication::Submit(const OrderInfo& info) {
  auto modifiedInfo = std::optional<OrderInfo>();
  auto submissionInfo = [&] () -> const OrderInfo* {
    if(info.m_fields.m_type != OrderType::MARKET) {
      return &info;
    }
    modifiedInfo.emplace(info);
    modifiedInfo->m_fields.m_type = OrderType::LIMIT;
    auto bboQuote = LoadBboQuote(modifiedInfo->m_fields.m_security);
    if(info.m_fields.m_side == Side::BID) {
      modifiedInfo->m_fields.m_price =
        bboQuote.m_ask.m_price + 2 * Money::CENT;
    } else {
      modifiedInfo->m_fields.m_price =
        std::max(bboQuote.m_bid.m_price - 2 * Money::CENT, Money::CENT / 2);
    }
    return &*modifiedInfo;
  }();
  return m_orderLog.Submit(*submissionInfo, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      if(submissionInfo->m_fields.m_security.GetCountry() !=
          DefaultCountries::CA()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid country."));
      }
      if(submissionInfo->m_fields.m_currency != DefaultCurrencies::CAD()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid currency."));
      }
      newOrderSingle->setField(UMIR_ACCOUNT_TYPE_TAG, "CL");
      newOrderSingle->setField(UMIR_USER_ID_TAG, GetUmirUserID());
      auto noTradeFeat = GetNoTradeFeat();
      auto noTradeKey = GetNoTradeKey();
      if(!noTradeFeat.empty() && !noTradeKey.empty()) {
        newOrderSingle->setField(NO_TRADE_FEAT_TAG, noTradeFeat);
        newOrderSingle->setField(NO_TRADE_KEY_TAG, noTradeKey);
      }
      newOrderSingle->set(
        FIX::Account(submissionInfo->m_submissionAccount.m_name));
      auto exDestination = [&] {
        if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::TSX()) {
          if(submissionInfo->m_fields.m_security.GetMarket() ==
              DefaultMarkets::TSXV()) {
            return FIX::ExDestination("TSXV");
          }
          return FIX::ExDestination("TSX");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::CHIX()) {
          return FIX::ExDestination("CHIX");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::CX2()) {
          return FIX::ExDestination("XCX2");
        } else if(
            submissionInfo->m_fields.m_destination ==
              DefaultDestinations::MATNLP()) {
          return FIX::ExDestination("MATN");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::CSE()) {
          return FIX::ExDestination("XCNQ");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::CSE2()) {
          return FIX::ExDestination("CSE2");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::ALPHA()) {
          return FIX::ExDestination("XATS");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::OMEGA()) {
          return FIX::ExDestination("OMGA");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::LYNX()) {
          return FIX::ExDestination("LYNX");
        } else if(submissionInfo->m_fields.m_destination ==
            DefaultDestinations::NEOE()) {
          return FIX::ExDestination("NEOL");
        }
        BOOST_THROW_EXCEPTION(
          FixOrderRejectedException("Invalid destination."));
      }();
      newOrderSingle->set(exDestination);
      for(auto& tag : submissionInfo->m_fields.m_additionalFields) {
        if(tag.GetKey() == LONG_LIFE_TAG) {
          if(auto value = get<std::string>(&tag.GetValue())) {
            if(*value == "Y" || *value == "N") {
              newOrderSingle->setField(LONG_LIFE_TAG, *value);
            }
          }
        }
      }
      if(submissionInfo->m_fields.m_destination ==
          DefaultDestinations::MATNLP() ||
            submissionInfo->m_fields.m_destination ==
              DefaultDestinations::MATNMF()) {
        auto constraintsTagIterator = std::find_if(
          submissionInfo->m_fields.m_additionalFields.begin(),
          submissionInfo->m_fields.m_additionalFields.end(),
          [] (const Tag& tag) {
            return tag.GetKey() == MATN_CONSTRAINTS_TAG;
          });
        if(constraintsTagIterator !=
            submissionInfo->m_fields.m_additionalFields.end()) {
          auto& constraintsTag = *constraintsTagIterator;
          auto value = get<std::string>(constraintsTag.GetValue());
          if(value == "PAG") {
            newOrderSingle->setField(MATN_CONSTRAINTS_TAG, "PAG=-1");
            if(submissionInfo->m_fields.m_destination == "MATNLP") {
              newOrderSingle->setField(FIX::ExecInst("R"));
            }
          } else if(value == "PMI") {
            newOrderSingle->setField(MATN_CONSTRAINTS_TAG, "PMI=1");
            if(submissionInfo->m_fields.m_destination == "MATNLP") {
              newOrderSingle->setField(FIX::ExecInst("p"));
            }
          }
        } else if(submissionInfo->m_fields.m_destination == "MATNLP") {
          newOrderSingle->setField(FIX::ExecInst("M"));
        }
      }
    });
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

BboQuote SerenityFixApplication::LoadBboQuote(const Security& security) {
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

std::string SerenityFixApplication::GetUmirUserID() const {
  if(GetSessionSettings().get(GetSessionId()).has("UMIRUserID")) {
    return GetSessionSettings().get(GetSessionId()).getString("UMIRUserID");
  }
  return {};
}

std::string SerenityFixApplication::GetNoTradeFeat() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeFeat")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeFeat");
  }
  return {};
}

std::string SerenityFixApplication::GetNoTradeKey() const {
  if(GetSessionSettings().get(GetSessionId()).has("NoTradeKey")) {
    return GetSessionSettings().get(GetSessionId()).getString("NoTradeKey");
  }
  return {};
}
