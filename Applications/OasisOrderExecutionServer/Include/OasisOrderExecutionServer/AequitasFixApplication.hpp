#ifndef OASIS_AEQUITASFIXAPPLICATION_HPP
#define OASIS_AEQUITASFIXAPPLICATION_HPP
#include <Beam/Network/Network.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <quickfix/MessageCracker.h>
#include "Nexus/FixUtilities/FixApplication.hpp"
#include "Nexus/FixUtilities/FixOrderLog.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OasisOrderExecutionService {

  /*! \class AequitasFixApplication
      \brief Implements a FIX Application using Aequitas.
   */
  class AequitasFixApplication : public FixUtilities::FixApplication,
      public FIX::MessageCracker {
    public:

      //! Constructs a AequitasFixApplication.
      /*!
        \param timeClient The TimeClient used for timestamps.
      */
      AequitasFixApplication(Beam::RefType<Beam::TimeService::LiveNtpTimeClient>
        timeClient);

      virtual const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& orderRecord);

      virtual const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& info);

      virtual void Cancel(
        const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId);

      virtual void Update(
        const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport);

      virtual void onCreate(const FIX::SessionID&);

      virtual void onLogon(const FIX::SessionID& sessionID);

      virtual void onLogout(const FIX::SessionID& sessionID);

      virtual void toAdmin(FIX::Message&, const FIX::SessionID&);

      virtual void toApp(FIX::Message&, const FIX::SessionID&) throw(
        FIX::DoNotSend);

      virtual void fromAdmin(const FIX::Message&, const FIX::SessionID&) throw(
        FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
        FIX::RejectLogon);

      virtual void fromApp(const FIX::Message& message,
        const FIX::SessionID& sessionID) throw(FIX::FieldNotFound,
        FIX::IncorrectDataFormat, FIX::IncorrectTagValue,
        FIX::UnsupportedMessageType);

      virtual void onMessage(const FIX42::ExecutionReport& message,
        const FIX::SessionID& sessionId);

      virtual void onMessage(const FIX42::TradingSessionStatus& message,
        const FIX::SessionID& sessionId);

      virtual void onMessage(const FIX42::OrderCancelReject& message,
        const FIX::SessionID& sessionId);

    private:
      Beam::TimeService::LiveNtpTimeClient* m_timeClient;
      FixUtilities::FixOrderLog m_orderLog;

      std::string GetAccount() const;
      std::string GetUmirUserID() const;
      std::string GetNoTradeFeat() const;
      std::string GetNoTradeKey() const;
  };
}
}

#endif
