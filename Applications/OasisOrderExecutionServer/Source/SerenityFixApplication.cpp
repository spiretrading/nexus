#include "OasisOrderExecutionServer/SerenityFixApplication.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include <quickfix/Session.h>

using namespace boost;
using namespace Beam;
using namespace Beam::TimeService;
using namespace Nexus;
using namespace Nexus::FixUtilities;
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto MATN_CONSTRAINTS_TAG = 6005;
  const auto UMIR_ACCOUNT_TYPE_TAG = 6750;
  const auto UMIR_USER_ID_TAG = 6751;
  const auto NO_TRADE_FEAT_TAG = 7713;
  const auto NO_TRADE_KEY_TAG = 7714;
  const auto LONG_LIFE_TAG = 7735;
  const auto UNIFORM_LIQUDITY_TAG = 9730;
  const auto ORIGINAL_LIQUIDITY_TAG = 9731;
}

SerenityFixApplication::SerenityFixApplication(
  Ref<LiveNtpTimeClient> timeClient)
  : m_timeClient(timeClient.Get()) {}

const Order& SerenityFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& SerenityFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_security.GetCountry() != DefaultCountries::CA()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid country."));
      }
      if(info.m_fields.m_currency != DefaultCurrencies::CAD()) {
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
      newOrderSingle->set(FIX::Account(info.m_submissionAccount.m_name));
      auto exDestination = [&] {
        if(info.m_fields.m_destination == DefaultDestinations::TSX()) {
          if(info.m_fields.m_security.GetMarket() == DefaultMarkets::TSXV()) {
            return FIX::ExDestination("TSXV");
          }
          return FIX::ExDestination("XTSX");
        } else if(info.m_fields.m_destination == DefaultDestinations::CHIX()) {
          return FIX::ExDestination("CHIX");
        } else if(info.m_fields.m_destination == DefaultDestinations::CX2()) {
          return FIX::ExDestination("XCX2");
        } else if(
            info.m_fields.m_destination == DefaultDestinations::MATNLP()) {
          return FIX::ExDestination("MATN");
        } else if(info.m_fields.m_destination == DefaultDestinations::CSE() ||
            info.m_fields.m_destination == DefaultDestinations::PURE()) {
          return FIX::ExDestination("XCNQ");
        } else if(info.m_fields.m_destination == DefaultDestinations::CSE2()) {
          return FIX::ExDestination("CSE2");
        } else if(info.m_fields.m_destination == DefaultDestinations::ALPHA()) {
          return FIX::ExDestination("XATS");
        } else if(info.m_fields.m_destination == DefaultDestinations::OMEGA()) {
          return FIX::ExDestination("OMGA");
        } else if(info.m_fields.m_destination == DefaultDestinations::LYNX()) {
          return FIX::ExDestination("LYNX");
        } else if(info.m_fields.m_destination == DefaultDestinations::NEOE()) {
          return FIX::ExDestination("NEOL");
        }
        BOOST_THROW_EXCEPTION(
          FixOrderRejectedException("Invalid destination."));
      }();
      newOrderSingle->set(exDestination);
      for(auto& tag : info.m_fields.m_additionalFields) {
        if(tag.GetKey() == LONG_LIFE_TAG) {
          if(auto value = get<std::string>(&tag.GetValue())) {
            if(*value == "Y" || *value == "N") {
              newOrderSingle->setField(LONG_LIFE_TAG, *value);
            }
          }
        }
      }
      if(info.m_fields.m_destination == DefaultDestinations::MATNLP() ||
          info.m_fields.m_destination == DefaultDestinations::MATNMF()) {
        auto constraintsTagIterator = std::find_if(
          info.m_fields.m_additionalFields.begin(),
          info.m_fields.m_additionalFields.end(),
          [] (const Tag& tag) {
            return tag.GetKey() == MATN_CONSTRAINTS_TAG;
          });
        if(constraintsTagIterator != info.m_fields.m_additionalFields.end()) {
          auto& constraintsTag = *constraintsTagIterator;
          auto value = get<std::string>(constraintsTag.GetValue());
          if(value == "PAG") {
            newOrderSingle->setField(MATN_CONSTRAINTS_TAG, "PAG=-1");
            if(info.m_fields.m_destination == "MATNLP") {
              newOrderSingle->setField(FIX::ExecInst("R"));
            }
          } else if(value == "PMI") {
            newOrderSingle->setField(MATN_CONSTRAINTS_TAG, "PMI=1");
            if(info.m_fields.m_destination == "MATNLP") {
              newOrderSingle->setField(FIX::ExecInst("p"));
            }
          }
        } else if(info.m_fields.m_destination == "MATNLP") {
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
