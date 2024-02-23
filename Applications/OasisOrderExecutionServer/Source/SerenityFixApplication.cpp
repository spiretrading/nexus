#include "OasisOrderExecutionServer/SerenityFixApplication.hpp"
#include <quickfix/Session.h>
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Beam;
using namespace Beam::TimeService;
using namespace Nexus;
using namespace Nexus::FixUtilities;
using namespace Nexus::MarketDataService;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto ANONYMOUS_TAG = 6761;
  const auto MATN_CONSTRAINTS_TAG = 6005;
  const auto UMIR_ACCOUNT_TYPE_TAG = 6750;
  const auto UMIR_USER_ID_TAG = 6751;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
  const auto LONG_LIFE_TAG = 7735;
  const auto UNIFORM_LIQUDITY_TAG = 9730;
  const auto ORIGINAL_LIQUIDITY_TAG = 9731;
  const auto NEO_VISIBILITY_TYPE_TAG = 20000;
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
    if(info.m_fields.m_type != OrderType::MARKET &&
        info.m_fields.m_type != OrderType::PEGGED) {
      return &info;
    }
    if(info.m_fields.m_type == OrderType::MARKET &&
        (info.m_fields.m_timeInForce.GetType() ==
          TimeInForce::Type::OPG ||
          info.m_fields.m_timeInForce.GetType() ==
            TimeInForce::Type::MOC)) {
      return &info;
    }
    modifiedInfo.emplace(info);
    if(info.m_fields.m_type == OrderType::MARKET) {
      modifiedInfo->m_fields.m_type = OrderType::LIMIT;
      modifiedInfo->m_fields.m_price = Money::ZERO;
    }
    auto bboQuote = LoadBboQuote(modifiedInfo->m_fields.m_security);
    if(modifiedInfo->m_fields.m_price == Money::ZERO) {
      if(info.m_fields.m_side == Side::BID) {
        modifiedInfo->m_fields.m_price =
          std::max(bboQuote.m_ask.m_price + 2 * Money::CENT,
            Floor(1.02 * bboQuote.m_ask.m_price, 2));
      } else {
        modifiedInfo->m_fields.m_price = std::max(
          std::min(bboQuote.m_bid.m_price - 2 * Money::CENT,
            Floor(0.98 * bboQuote.m_bid.m_price, 2)), Money::CENT / 2);
      }
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
      if(auto& anonymousTag = GetAnonymousTag()) {
        newOrderSingle->setField(ANONYMOUS_TAG, *anonymousTag);
      }
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
          DefaultDestinations::CHIX() ||
          submissionInfo->m_fields.m_destination ==
            DefaultDestinations::CX2() ||
          submissionInfo->m_fields.m_destination ==
            DefaultDestinations::TSX()) {
        RouteToChix(*submissionInfo, Store(newOrderSingle));
      } else if(submissionInfo->m_fields.m_destination ==
          DefaultDestinations::MATNLP() ||
          submissionInfo->m_fields.m_destination ==
            DefaultDestinations::MATNMF()) {
        RouteToMatn(*submissionInfo, Store(newOrderSingle));
      } else if(submissionInfo->m_fields.m_destination ==
          DefaultDestinations::NEOE()) {
        RouteToNeo(*submissionInfo, Store(newOrderSingle));
      } else {
        auto exDestination = [&] {
          if(submissionInfo->m_fields.m_destination ==
              DefaultDestinations::CSE() ||
                submissionInfo->m_fields.m_destination ==
                  DefaultDestinations::PURE()) {
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
          }
          BOOST_THROW_EXCEPTION(
            FixOrderRejectedException("Invalid destination."));
        }();
        newOrderSingle->set(FIX::HandlInst('6'));
        newOrderSingle->set(exDestination);
      }
    });
}

void SerenityFixApplication::Cancel(
    const OrderExecutionSession& session, OrderId orderId) {
  if(!m_cancellations.Insert(orderId)) {
    return;
  }
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->setField(UMIR_USER_ID_TAG, GetUmirUserID());
    });
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
    const FIX42::ExecutionReport& message, const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      auto liquidityFlag = std::string();
      if(message.isSetField(UNIFORM_LIQUDITY_TAG)) {
        liquidityFlag = message.getField(UNIFORM_LIQUDITY_TAG);
      }
      if(liquidityFlag == "A") {
        update->m_liquidityFlag = "P";
      } else if(liquidityFlag == "R") {
        update->m_liquidityFlag = "A";
      }
      auto lastMkt = FIX::LastMkt();
      if(message.isSet(lastMkt)) {
        message.get(lastMkt);
        if(lastMkt == "XTSX") {
          update->m_lastMarket = DefaultMarkets::TSX().GetData();
        } else if(lastMkt == "CHIX" || lastMkt == "XCXD") {
          update->m_lastMarket = DefaultMarkets::CHIC().GetData();
        } else if(lastMkt == "XCX2") {
          update->m_lastMarket = DefaultMarkets::XCX2().GetData();
        } else if(lastMkt == "MATN") {
          update->m_lastMarket = DefaultMarkets::MATN().GetData();
        } else if(lastMkt == "XCNQ") {
          if(order.GetInfo().m_fields.m_security.GetMarket() ==
              DefaultMarkets::CSE()) {
            update->m_lastMarket = DefaultMarkets::CSE().GetData();
          } else {
            update->m_lastMarket = DefaultMarkets::PURE().GetData();
          }
        } else if(lastMkt == "CSE2") {
          update->m_lastMarket = DefaultMarkets::CSE2().GetData();
        } else if(lastMkt == "XATS") {
          update->m_lastMarket = DefaultMarkets::XATS().GetData();
        } else if(lastMkt == "OMGA") {
          update->m_lastMarket = DefaultMarkets::OMGA().GetData();
        } else if(lastMkt == "LYNX") {
          update->m_lastMarket = DefaultMarkets::LYNX().GetData();
        } else if(lastMkt == "NEON" || lastMkt == "NEOL" || lastMkt == "NEOD") {
          update->m_lastMarket = DefaultMarkets::NEOE().GetData();
        } else if(lastMkt == "TSXV") {
          update->m_lastMarket = DefaultMarkets::TSXV().GetData();
        }
      }
  });
}

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
      FixOrderRejectedException("No BBO quote available."));
  }
}

void SerenityFixApplication::RouteToChix(
    OrderInfo info, Out<FIX42::NewOrderSingle> newOrderSingle) {
  auto hasDestination = false;
  if(info.m_fields.m_destination == DefaultDestinations::TSX()) {
    static const auto openTime = hours(14) + minutes(30);
    if(m_timeClient->GetTime().time_of_day() < openTime) {
      auto destination = [&] {
        if(info.m_fields.m_security.GetMarket() == DefaultMarkets::TSXV()) {
          return FIX::ExDestination("TSXV");
        }
        return FIX::ExDestination("XTSX");
      }();
      newOrderSingle->set(FIX::HandlInst('6'));
      newOrderSingle->getHeader().setField(destination);
      if(info.m_fields.m_timeInForce.GetType() ==
          TimeInForce::Type::DAY) {
        info.m_fields.m_timeInForce = TimeInForce(TimeInForce::Type::OPG);
      }
    } else {
      auto destination = FIX::ExDestination("CX25");
      newOrderSingle->getHeader().setField(destination);
      if(info.m_fields.m_type == OrderType::PEGGED) {
        newOrderSingle->set(FIX::ExecInst("M"));
      }
    }
    hasDestination = true;
  } else {
    for(auto& tag : info.m_fields.m_additionalFields) {
      if(tag.GetKey() == FIX::FIELD::ExDestination) {
        if(auto value = boost::get<std::string>(&tag.GetValue())) {
          auto destination = [&] {
            if(*value == "SMRTFEE") {
              return FIX::ExDestination("CX15");
            } else if(*value == "SMRTXDARKNR") {
              return FIX::ExDestination("CX11");
            } else if(*value == "CXD") {
              return FIX::ExDestination("XCXD");
            } else if(*value == "SMRTCXD") {
              return FIX::ExDestination("XCXD");
            }
            BOOST_THROW_EXCEPTION(FixOrderRejectedException(
              "Invalid value for tag 100 (ExDestination)."));
          }();
          newOrderSingle->getHeader().setField(destination);
          if(*value == "SMRTXDARKNR") {
            newOrderSingle->setField(ANONYMOUS_TAG, "Y");
          }
          hasDestination = true;
        } else {
          BOOST_THROW_EXCEPTION(FixOrderRejectedException(
            "Invalid value for tag 100 (ExDestination)."));
        }
      }
    }
  }
  if(!hasDestination) {
    if(info.m_fields.m_type == OrderType::PEGGED) {
      auto destination = [&] {
        if(info.m_fields.m_destination == DefaultDestinations::CHIX()) {
          return FIX::ExDestination("CHIX");
        } else if(info.m_fields.m_destination ==
            DefaultDestinations::CX2()) {
          return FIX::ExDestination("XCX2");
        } else {
          BOOST_THROW_EXCEPTION(
            FixOrderRejectedException("Destination not supported."));
        }
      }();
      newOrderSingle->getHeader().setField(destination);
    } else {
      auto destination = [&] {
        if(info.m_fields.m_destination == DefaultDestinations::CHIX()) {
          return FIX::ExDestination("CX01");
        } else if(info.m_fields.m_destination ==
            DefaultDestinations::CX2()) {
          return FIX::ExDestination("CX05");
        } else {
          BOOST_THROW_EXCEPTION(
            FixOrderRejectedException("Destination not supported."));
        }
      }();
      newOrderSingle->getHeader().setField(destination);
    }
  }
}

void SerenityFixApplication::RouteToMatn(
    const OrderInfo& info, Out<FIX42::NewOrderSingle> newOrderSingle) {
  newOrderSingle->setField(FIX::ExDestination("MATN"));
  auto constraintsTagIterator = std::find_if(
    info.m_fields.m_additionalFields.begin(),
    info.m_fields.m_additionalFields.end(),
    [] (const auto & tag) {
      return tag.GetKey() == MATN_CONSTRAINTS_TAG;
    });
  if(constraintsTagIterator != info.m_fields.m_additionalFields.end()) {
    auto& constraintsTag = *constraintsTagIterator;
    auto value = get<std::string>(constraintsTag.GetValue());
    if(value == "PAG") {
      newOrderSingle->setField(FIX::ExecInst("R"));
      newOrderSingle->setField(FIX::OrdType(FIX::OrdType_PEGGED));
    } else if(value == "PMI") {
      newOrderSingle->setField(FIX::ExecInst("p"));
      newOrderSingle->setField(FIX::OrdType(FIX::OrdType_PEGGED));
    }
  } else if(info.m_fields.m_destination == "MATNLP") {
    newOrderSingle->setField(FIX::ExecInst("M"));
    newOrderSingle->setField(FIX::OrdType(FIX::OrdType_PEGGED));
  }
}

void SerenityFixApplication::RouteToNeo(
    const OrderInfo& info, Out<FIX42::NewOrderSingle> newOrderSingle) {
  auto isProtected = true;
  auto isMidPoint = false;
  auto isNeoBook = false;
  for(auto& tag : info.m_fields.m_additionalFields) {
    if(tag.GetKey() == FIX::FIELD::ExecInst) {
      if(auto value = get<std::string>(&tag.GetValue())) {
        if(*value == "M") {
          isProtected = false;
          isMidPoint = true;
        }
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException(
          "Invalid value for tag 18 (ExecInst)."));
      }
    } else if(tag.GetKey() == FIX::FIELD::ExDestination) {
      if(auto value = get<std::string>(&tag.GetValue())) {
        auto destination = [&] {
          if(*value == "N") {
            return FIX::ExDestination("NEON");
          }
          BOOST_THROW_EXCEPTION(FixOrderRejectedException(
            "Invalid value for tag 100 (ExDestination)."));
        }();
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
  if(!isNeoBook) {
    newOrderSingle->getHeader().setField(FIX::ExDestination("NEOL"));
  }
  if(isProtected) {
    newOrderSingle->set(FIX::HandlInst('5'));
  }
  if(isMidPoint) {
    if(!isNeoBook) {
      newOrderSingle->setField(NEO_VISIBILITY_TYPE_TAG, "2");
    }
  }
}

const optional<std::string>& SerenityFixApplication::GetAnonymousTag() const {
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
