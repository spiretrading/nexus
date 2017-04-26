#ifndef NEXUS_FIX44ORDERLOG_HPP
#define NEXUS_FIX44ORDERLOG_HPP
#include <string>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/SynchronizedList.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include <quickfix/fix44/ExecutionReport.h>
#include <quickfix/fix44/NewOrderSingle.h>
#include <quickfix/fix44/OrderCancelRequest.h>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/FixUtilities/FixConversions.hpp"
#include "Nexus/FixUtilities/FixOrder.hpp"
#include "Nexus/FixUtilities/FixOrderRejectedException.hpp"
#include "Nexus/FixUtilities/FixUtilities.hpp"

namespace Nexus {
namespace FixUtilities {

  /*! \class Fix44OrderLog
      \brief Used to create and update FIX Orders.
   */
  class Fix44OrderLog : private boost::noncopyable {
    public:

      //! Returns an Order's id from a FIX ExecutionReport message.
      /*!
        \param message The message to extract the Order id from.
        \return The Order id represented by the <i>message</i>.
      */
      static boost::optional<OrderExecutionService::OrderId> GetOrderId(
        const FIX44::ExecutionReport& message);

      //! Returns information about a trade/fill from a FIX ExecutionReport
      //! message.
      /*!
        \param message The message to extract the fill info from.
        \param lastQuantity Stores the quantity of the last fill.
        \param lastPrice Stores the price of the last fill.
      */
      static void GetFillInfo(const FIX44::ExecutionReport& message,
        Beam::Out<Quantity> lastQuantity, Beam::Out<Money> lastPrice);

      //! Constructs a Fix44OrderLog.
      Fix44OrderLog() = default;

      //! Returns the Order with a specified id.
      /*!
        \param orderId The Order id to find.
        \return The Order with the specified <i>orderId</i> or <code>null</code>
                if no such Order exists.
      */
      std::shared_ptr<OrderExecutionService::PrimitiveOrder> FindOrder(
        OrderExecutionService::OrderId orderId) const;

      //! Recovers a previously unlogged Order.
      /*!
        \param orderRecord The OrderRecord to recover.
        \return The recovered Order.
      */
      const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& orderRecord);

      //! Submits a NewOrderSingle message.
      /*!
        \param session The session submitting the Order.
        \param orderId The Order's id.
        \param fields The OrderFields to submit.
        \param shortingFlag Whether the submission is a short sale.
        \param timestamp The submission's timestamp.
        \param senderCompId The session's SenderCompID.
        \param targetCompId The session's TargetCompID.
        \param f Receives the NewOrderSingle message to be submitted.
        \return The Order that was submitted.
      */
      template<typename F>
      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::OrderFields& fields, bool shortingFlag,
        const boost::posix_time::ptime& timestamp,
        const FIX::SenderCompID& senderCompId,
        const FIX::TargetCompID targetCompId, F f);

      //! Submits a OrderCancelRequest message.
      /*!
        \param session The session submitting the Order.
        \param orderId The Order id to cancel.
        \param timestamp The pending cancel's timestamp.
        \param senderCompId The session's SenderCompID.
        \param targetCompId The session's TargetCompID.
        \param f Receives the OrderCancelRequest message to be submitted.
      */
      template<typename F>
      void Cancel(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const boost::posix_time::ptime& timestamp,
        const FIX::SenderCompID& senderCompId,
        const FIX::TargetCompID targetCompId, F f);

      //! Updates an Order based on a FIX ExecutionReport message.
      /*!
        \param sessionId The session's id.
        \param timestamp The update's timestamp.
        \param f Receives the ExecutionReport that will be used to update the
                 Order.
      */
      template<typename F>
      void Update(const FIX44::ExecutionReport& message,
        const FIX::SessionID& sessionId,
        const boost::posix_time::ptime& timestamp, F f);

    private:
      struct RecoveredExecutionReport {
        FIX44::ExecutionReport m_message;
        std::function<void (Beam::Out<OrderExecutionService::ExecutionReport>)>
          m_callback;

        RecoveredExecutionReport(const FIX44::ExecutionReport& message,
          const std::function<
          void (Beam::Out<OrderExecutionService::ExecutionReport>)>& callback);
      };
      Beam::Threading::Sync<std::unordered_map<OrderExecutionService::OrderId,
        std::shared_ptr<OrderExecutionService::PrimitiveOrder>>> m_orders;
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId,
        Beam::SynchronizedVector<RecoveredExecutionReport>>
        m_recoveredExecutionReports;

      std::shared_ptr<OrderExecutionService::PrimitiveOrder> AddOrder(
        OrderExecutionService::OrderId id,
        const OrderExecutionService::OrderFields& fields, FIX::Side side,
        const boost::posix_time::ptime& timestamp);
      std::shared_ptr<OrderExecutionService::PrimitiveOrder> AddOrder(
        OrderExecutionService::OrderId id,
        const OrderExecutionService::OrderFields& fields, FIX::Side side,
        const std::vector<OrderExecutionService::ExecutionReport>&
        executionReports);
      std::shared_ptr<OrderExecutionService::PrimitiveOrder> RejectOrder(
        OrderExecutionService::OrderId id,
        const OrderExecutionService::OrderFields& fields,
        const boost::posix_time::ptime& timestamp, const std::string& reason);
      template<typename F>
      void Update(const FIX44::ExecutionReport& message,
        const boost::posix_time::ptime& timestamp,
        const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
        F f);
  };

  inline Fix44OrderLog::RecoveredExecutionReport::RecoveredExecutionReport(
      const FIX44::ExecutionReport& message, const std::function<
      void (Beam::Out<OrderExecutionService::ExecutionReport>)>& callback)
      : m_message(message),
        m_callback(callback) {}

  inline boost::optional<OrderExecutionService::OrderId> Fix44OrderLog::
      GetOrderId(const FIX44::ExecutionReport& message) {
    FIX::ClOrdID clOrdID;
    message.get(clOrdID);
    auto baseClOrdID = clOrdID.getString();
    auto delimiter = baseClOrdID.find('-');
    OrderExecutionService::OrderId orderId;
    try {
      if(delimiter == std::string::npos) {
        orderId = boost::lexical_cast<OrderExecutionService::OrderId>(
          baseClOrdID);
      } else {
        orderId = boost::lexical_cast<OrderExecutionService::OrderId>(
          baseClOrdID.substr(0, delimiter));
      }
    } catch(const boost::bad_lexical_cast&) {
      return boost::optional<OrderExecutionService::OrderId>();
    }
    return orderId;
  }

  inline void Fix44OrderLog::GetFillInfo(const FIX44::ExecutionReport& message,
      Beam::Out<Quantity> lastQuantity, Beam::Out<Money> lastPrice) {
    if(!message.isSetField(FIX::FIELD::LastQty) ||
        !message.isSetField(FIX::FIELD::LastPx)) {
      *lastQuantity = 0;
      *lastPrice = Money::ZERO;
      return;
    }
    FIX::LastQty lastQtyField;
    message.get(lastQtyField);
    try {
      *lastQuantity = boost::lexical_cast<Quantity>(lastQtyField.getValue());
    } catch(const boost::bad_lexical_cast&) {
      *lastQuantity = 0;
      *lastPrice = Money::ZERO;
      return;
    }
    FIX::LastPx lastPxField;
    message.get(lastPxField);
    auto lastPxFieldValue = Money::FromValue(lastPxField.getString());
    if(lastPxFieldValue.is_initialized()) {
      *lastPrice = *lastPxFieldValue;
    } else {
      *lastQuantity = 0;
      *lastPrice = Money::ZERO;
      return;
    }
  }

  inline std::shared_ptr<OrderExecutionService::PrimitiveOrder>
      Fix44OrderLog::FindOrder(OrderExecutionService::OrderId id) const {
    return Beam::Threading::With(m_orders,
      [&] (const std::unordered_map<OrderExecutionService::OrderId,
          std::shared_ptr<OrderExecutionService::PrimitiveOrder>>& orders) ->
          std::shared_ptr<OrderExecutionService::PrimitiveOrder> {
        auto orderIterator = orders.find(id);
        if(orderIterator == orders.end()) {
          return nullptr;
        }
        return orderIterator->second;
      });
  }

  inline const OrderExecutionService::Order& Fix44OrderLog::Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    if((*orderRecord)->m_executionReports.empty()) {
      auto initialExecutionReport =
        OrderExecutionService::ExecutionReport::BuildInitialReport(
        (*orderRecord)->m_orderId, (*orderRecord)->m_timestamp);
      auto properOrderRecord = orderRecord;
      (*properOrderRecord)->m_executionReports.push_back(
        initialExecutionReport);
      return Recover(properOrderRecord);
    }
    auto side = GetSide((*orderRecord)->m_fields.m_side,
      (*orderRecord)->m_shortingFlag);
    if(!side.is_initialized()) {
      BOOST_THROW_EXCEPTION(
        OrderExecutionService::OrderUnrecoverableException());
    }
    auto order = AddOrder((*orderRecord)->m_orderId, (*orderRecord)->m_fields,
      *side, (*orderRecord)->m_executionReports);
    auto recoveredExecutionReports = m_recoveredExecutionReports.Find(
      (*orderRecord)->m_orderId);
    if(recoveredExecutionReports.is_initialized()) {
      recoveredExecutionReports->ForEach(
        [&] (const RecoveredExecutionReport& recoveredExecutionReport) {
          FIX::UtcTimeStamp fixTimestamp;
          if(recoveredExecutionReport.m_message.isSetField(
              FIX::FIELD::TransactTime)) {
            FIX::TransactTime transactionTime;
            recoveredExecutionReport.m_message.get(transactionTime);
            fixTimestamp = static_cast<FIX::UtcTimeStamp>(transactionTime);
          } else {
            FIX::SendingTime sendingTime;
            recoveredExecutionReport.m_message.getHeader().get(sendingTime);
            fixTimestamp = static_cast<FIX::UtcTimeStamp>(sendingTime);
          }
          auto timestamp = GetTimestamp(fixTimestamp);
          Update(recoveredExecutionReport.m_message, timestamp, order,
            recoveredExecutionReport.m_callback);
        });
    }
    return *order;
  }

  template<typename F>
  const OrderExecutionService::Order& Fix44OrderLog::Submit(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::OrderFields& fields, bool shortingFlag,
      const boost::posix_time::ptime& timestamp,
      const FIX::SenderCompID& senderCompId,
      const FIX::TargetCompID targetCompId, F f) {
    auto ordType = GetOrderType(fields.m_type);
    if(!ordType.is_initialized()) {
      return *RejectOrder(orderId, fields, timestamp, "Invalid order type.");
    }
    auto side = GetSide(fields.m_side, shortingFlag);
    if(!side.is_initialized()) {
      return *RejectOrder(orderId, fields, timestamp, "Invalid side.");
    }
    FIX44::NewOrderSingle newOrderSingle;
    FIX::ClOrdID clOrdId(boost::lexical_cast<std::string>(orderId));
    newOrderSingle.set(FIX::HandlInst('1'));
    newOrderSingle.set(clOrdId);
    FIX::Symbol symbol(fields.m_security.GetSymbol());
    newOrderSingle.set(symbol);
    newOrderSingle.set(*side);
    newOrderSingle.set(FIX::TransactTime(GetUtcTimestamp(timestamp)));
    newOrderSingle.set(*ordType);
    newOrderSingle.set(FIX::OrderQty(static_cast<FIX::QTY>(fields.m_quantity)));
    auto timeInForceType = GetTimeInForceType(fields.m_timeInForce.GetType());
    if(!timeInForceType.is_initialized()) {
      return *RejectOrder(orderId, fields, timestamp, "Invalid time in force.");
    }
    newOrderSingle.set(*timeInForceType);
    if(fields.m_type == OrderType::LIMIT ||
        (fields.m_type == OrderType::PEGGED && fields.m_price != Money::ZERO)) {
      newOrderSingle.set(FIX::Price(ToDouble(fields.m_price)));
    }
    AddAdditionalTags(fields.m_additionalFields, Beam::Store(newOrderSingle));
    try {
      f(Beam::Store(newOrderSingle));
    } catch(const std::exception& e) {
      return *RejectOrder(orderId, fields, timestamp, e.what());
    }
    FIX::Session::sendToTarget(newOrderSingle, senderCompId, targetCompId);
    auto order = AddOrder(orderId, fields, *side, timestamp);
    return *order;
  }

  template<typename F>
  void Fix44OrderLog::Cancel(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const boost::posix_time::ptime& timestamp,
      const FIX::SenderCompID& senderCompId,
      const FIX::TargetCompID targetCompId, F f) {
    auto order = std::dynamic_pointer_cast<FixOrder>(FindOrder(orderId));
    if(order == nullptr) {
      return;
    }
    FIX::OrigClOrdID origClOrdID(boost::lexical_cast<std::string>(orderId));
    auto clOrdId = order->GetNextCancelId();
    FIX44::OrderCancelRequest orderCancelRequest(origClOrdID, clOrdId,
      order->GetSide(), FIX::TransactTime());
    FIX::OrderQty orderQty(
      static_cast<FIX::QTY>(order->GetFields().m_quantity));
    orderCancelRequest.set(orderQty);
    f(static_cast<const OrderExecutionService::Order&>(*order),
      Beam::Store(orderCancelRequest));
    auto sendMessage = false;
    order->With(
      [&] (OrderStatus status,
          const std::vector<OrderExecutionService::ExecutionReport>& reports) {
        if(IsTerminal(status) || reports.empty()) {
          return;
        }
        auto pendingCancelReport =
          OrderExecutionService::ExecutionReport::BuildUpdatedReport(
          reports.back(), OrderStatus::PENDING_CANCEL, timestamp);
        order->Update(pendingCancelReport);
        sendMessage = true;
      });
    if(sendMessage) {
      FIX::Session::sendToTarget(orderCancelRequest, senderCompId,
        targetCompId);
    }
  }

  template<typename F>
  void Fix44OrderLog::Update(const FIX44::ExecutionReport& message,
      const FIX::SessionID& sessionId,
      const boost::posix_time::ptime& timestamp, F f) {
    auto orderId = GetOrderId(message);
    if(!orderId.is_initialized()) {
      return;
    }

    // TODO: Turn this into a transaction to avoid possible race condition.
    auto order = FindOrder(*orderId);
    if(order == nullptr) {
      RecoveredExecutionReport recoveredExecutionReport(message,
        [f = std::move(f), order] (
            Beam::Out<OrderExecutionService::ExecutionReport>
            executionReport) {
          f(static_cast<const OrderExecutionService::Order&>(*order),
            Beam::Store(executionReport));
        });
      m_recoveredExecutionReports.Get(*orderId).PushBack(
        recoveredExecutionReport);
      return;
    }
    Update(message, timestamp, order, std::move(f));
  }

  inline std::shared_ptr<OrderExecutionService::PrimitiveOrder>
      Fix44OrderLog::AddOrder(OrderExecutionService::OrderId id,
      const OrderExecutionService::OrderFields& fields, FIX::Side side,
      const boost::posix_time::ptime& timestamp) {
    auto order = std::make_shared<FixOrder>(id, fields, side, timestamp);
    Beam::Threading::With(m_orders,
      [&] (std::unordered_map<OrderExecutionService::OrderId,
          std::shared_ptr<OrderExecutionService::PrimitiveOrder>>& orders) {
        orders.insert(std::make_pair(id, order));
      });
    return order;
  }

  inline std::shared_ptr<OrderExecutionService::PrimitiveOrder>
      Fix44OrderLog::AddOrder(OrderExecutionService::OrderId id,
      const OrderExecutionService::OrderFields& fields, FIX::Side side,
      const std::vector<OrderExecutionService::ExecutionReport>&
      executionReports) {
    auto order = std::make_shared<FixOrder>(id, fields,
      executionReports.front().m_timestamp, side, executionReports);
    Beam::Threading::With(m_orders,
      [&] (std::unordered_map<OrderExecutionService::OrderId,
          std::shared_ptr<OrderExecutionService::PrimitiveOrder>>& orders) {
        orders.insert(std::make_pair(id, order));
      });
    return order;
  }

  inline std::shared_ptr<OrderExecutionService::PrimitiveOrder>
      Fix44OrderLog::RejectOrder(OrderExecutionService::OrderId id,
      const OrderExecutionService::OrderFields& fields,
      const boost::posix_time::ptime& timestamp, const std::string& reason) {
    std::shared_ptr<OrderExecutionService::PrimitiveOrder> order =
      BuildRejectedOrder(id, fields, timestamp, reason);
    Beam::Threading::With(m_orders,
      [&] (std::unordered_map<OrderExecutionService::OrderId,
          std::shared_ptr<OrderExecutionService::PrimitiveOrder>>& orders) {
        orders.insert(std::make_pair(id, order));
      });
    return order;
  }

  template<typename F>
  void Fix44OrderLog::Update(const FIX44::ExecutionReport& message,
      const boost::posix_time::ptime& timestamp,
      const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
      F f) {
    FIX::OrdStatus ordStatus;
    message.get(ordStatus);
    auto orderStatus = GetOrderStatus(ordStatus);
    if(!orderStatus.is_initialized() ||
        *orderStatus == OrderStatus::PENDING_NEW) {
      return;
    }
    auto isPendingCancel = *orderStatus == OrderStatus::PENDING_CANCEL;
    Quantity lastQuantity;
    Money lastPrice;
    GetFillInfo(message, Beam::Store(lastQuantity), Beam::Store(lastPrice));
    if(isPendingCancel && lastQuantity == 0) {
      return;
    }
    FIX::Text text;
    if(message.isSet(text)) {
      message.get(text);
    }
    order->With(
      [&] (OrderStatus status,
          const std::vector<OrderExecutionService::ExecutionReport>& reports) {
        if(reports.empty() || IsTerminal(reports.back().m_status)) {
          std::cout << "Stale Report: " << message.toString() << std::endl;
          return;
        }
        if(isPendingCancel) {
          auto filledQuantity = lastQuantity;
          for(const auto& report : reports) {
            filledQuantity += report.m_lastQuantity;
          }
          if(filledQuantity >= order->GetFields().m_quantity) {
            orderStatus = OrderStatus::FILLED;
          } else {
            orderStatus = OrderStatus::PARTIALLY_FILLED;
          }
        }
        auto updatedReport =
          OrderExecutionService::ExecutionReport::BuildUpdatedReport(
          reports.back(), *orderStatus, timestamp);
        updatedReport.m_lastQuantity = lastQuantity;
        updatedReport.m_lastPrice = lastPrice;
        updatedReport.m_text = text.getString();
        f(static_cast<const OrderExecutionService::Order&>(*order),
          Beam::Store(updatedReport));
        order->Update(updatedReport);
      });
  }
}
}

#endif
