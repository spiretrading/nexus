#include "OasisOrderExecutionServer/AsxFixApplication.hpp"
#include <boost/throw_exception.hpp>
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
using namespace Nexus::OasisOrderExecutionService;
using namespace Nexus::OrderExecutionService;
using namespace std;

namespace {
  const auto ACCOUNT_TAG = 28888;
}

AsxFixApplication::AsxFixApplication(Ref<LiveNtpTimeClient> timeClient)
    : m_timeClient(timeClient.Get()) {}

const Order& AsxFixApplication::Recover(
    const SequencedAccountOrderRecord& orderRecord) {
  return m_orderLog.Recover(orderRecord);
}

const Order& AsxFixApplication::Submit(const OrderInfo& info) {
  return m_orderLog.Submit(info, GetSessionId().getSenderCompID(),
    GetSessionId().getTargetCompID(),
    [&] (Out<FIX42::NewOrderSingle> newOrderSingle) {
      if(info.m_fields.m_timeInForce.GetType() == TimeInForce::Type::GTC ||
          info.m_fields.m_timeInForce.GetType() == TimeInForce::Type::GTD) {
        BOOST_THROW_EXCEPTION(
          FixOrderRejectedException("Invalid time in force."));
      }
      if(info.m_fields.m_type == OrderType::STOP) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException("Invalid order type."));
      }
      if(info.m_fields.m_security.GetCountry() != DefaultCountries::AU()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid country."});
      }
      if(info.m_fields.m_currency != DefaultCurrencies::AUD()) {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid currency."});
      }
      newOrderSingle->set(FIX::Account(GetAccount()));
      newOrderSingle->setField(ACCOUNT_TAG, info.m_submissionAccount.m_name);
      if(auto deliverToCompId = GetDeliverToCompId()) {
        newOrderSingle->getHeader().setField(
          FIX::DeliverToCompID(*deliverToCompId));
      }
      if(info.m_fields.m_security.GetMarket() == DefaultMarkets::ASX()) {
        newOrderSingle->set(FIX::SecurityExchange{"ASX"});
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid market."});
      }
      if(info.m_fields.m_destination == DefaultDestinations::ASXT()) {
        newOrderSingle->set(FIX::ExDestination{"BESTMKT"});
      } else if(info.m_fields.m_destination == DefaultDestinations::CXA()) {
        if(info.m_fields.m_type == OrderType::PEGGED) {
          newOrderSingle->set(FIX::ExDestination{"CXA"});
        } else {
          newOrderSingle->set(FIX::ExDestination{"BESTMKT2"});
        }
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{
          "Invalid destination."});
      }
      if(info.m_fields.m_type == OrderType::MARKET) {
        newOrderSingle->set(FIX::OrdType{'K'});
      }
    });
}

void AsxFixApplication::Cancel(const OrderExecutionSession& session,
    OrderId orderId) {
  m_orderLog.Cancel(session, orderId, m_timeClient->GetTime(),
    GetSessionId().getSenderCompID(), GetSessionId().getTargetCompID(),
    [&] (const Order& order,
        Out<FIX42::OrderCancelRequest> orderCancelRequest) {
      orderCancelRequest->set(FIX::Account(GetAccount()));
      orderCancelRequest->setField(ACCOUNT_TAG, session.GetAccount().m_name);
      if(auto deliverToCompId = GetDeliverToCompId()) {
        orderCancelRequest->getHeader().setField(
          FIX::DeliverToCompID(*deliverToCompId));
      }
      auto& fields = order.GetInfo().m_fields;
      if(fields.m_security.GetMarket() == DefaultMarkets::ASX()) {
        orderCancelRequest->set(FIX::SecurityExchange{"ASX"});
      } else {
        BOOST_THROW_EXCEPTION(FixOrderRejectedException{"Invalid market."});
      }
    });
}

void AsxFixApplication::Update(const OrderExecutionSession& session,
    OrderId orderId, const ExecutionReport& executionReport) {
  m_orderLog.Update(session, orderId, executionReport, m_timeClient->GetTime());
}

void AsxFixApplication::onCreate(const FIX::SessionID& sessionID) {}

void AsxFixApplication::onLogon(const FIX::SessionID& sessionID) {}

void AsxFixApplication::onLogout(const FIX::SessionID& sessionID) {}

void AsxFixApplication::toAdmin(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void AsxFixApplication::toApp(FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void AsxFixApplication::fromAdmin(const FIX::Message& message,
    const FIX::SessionID& sessionID) {}

void AsxFixApplication::fromApp(const FIX::Message& message,
    const FIX::SessionID& sessionID) {
  crack(message, sessionID);
}

void AsxFixApplication::onMessage(const FIX42::ExecutionReport& message,
    const FIX::SessionID& sessionId) {
  m_orderLog.Update(message, sessionId, m_timeClient->GetTime(),
    [=] (const Order& order, Out<ExecutionReport> update) {
      if(update->m_lastQuantity != 0) {
        update->m_liquidityFlag = lexical_cast<std::string>(
          LiquidityFlag::ACTIVE);
        auto lastMkt = FIX::LastMkt();
        if(message.isSet(lastMkt)) {
          message.get(lastMkt);
        }
        if(lastMkt == "CXA" || lastMkt == "CXAP" || lastMkt == "CXAC") {
          update->m_lastMarket = DefaultDestinations::CXA();
        } else if(lastMkt == "TM") {
          update->m_lastMarket = DefaultDestinations::ASXT();
        } else {
          update->m_lastMarket = order.GetInfo().m_fields.m_destination;
        }
      }
    });
}

void AsxFixApplication::onMessage(const FIX42::TradingSessionStatus& message,
    const FIX::SessionID& sessionId) {}

void AsxFixApplication::onMessage(const FIX42::OrderCancelReject& message,
    const FIX::SessionID& sessionId) {}

string AsxFixApplication::GetAccount() const {
  return GetSessionSettings().get(GetSessionId()).getString("Account");
}

string AsxFixApplication::GetUsername() const {
  return GetSessionSettings().get(GetSessionId()).getString("Username");
}

string AsxFixApplication::GetPassword() const {
  return GetSessionSettings().get(GetSessionId()).getString("Password");
}

boost::optional<string> AsxFixApplication::GetDeliverToCompId() const {
  auto& settings = GetSessionSettings().get(GetSessionId());
  if(settings.has("DeliverToCompID")) {
    return settings.getString("DeliverToCompID");
  }
  return none;
}
