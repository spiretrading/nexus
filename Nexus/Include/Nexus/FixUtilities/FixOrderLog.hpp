#ifndef NEXUS_FIXORDERLOG_HPP
#define NEXUS_FIXORDERLOG_HPP
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
#include <quickfix/fix42/ExecutionReport.h>
#include <quickfix/fix42/NewOrderSingle.h>
#include <quickfix/fix42/OrderCancelRequest.h>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/FixUtilities/FixConversions.hpp"
#include "Nexus/FixUtilities/FixOrder.hpp"
#include "Nexus/FixUtilities/FixOrderRejectedException.hpp"
#include "Nexus/FixUtilities/FixUtilities.hpp"

namespace Nexus {
namespace FixUtilities {

  /*! \class FixOrderLog
      \brief Used to create and update FIX Orders.
   */
  class FixOrderLog : private boost::noncopyable {
    public:

      //! Returns an Order's id from a FIX ExecutionReport message.
      /*!
        \param message The message to extract the Order id from.
        \return The Order id represented by the <i>message</i>.
      */
      static boost::optional<OrderExecutionService::OrderId> GetOrderId(
        const FIX42::ExecutionReport& message);

      //! Returns information about a trade/fill from a FIX ExecutionReport
      //! message.
      /*!
        \param message The message to extract the fill info from.
        \param lastQuantity Stores the quantity of the last fill.
        \param lastPrice Stores the price of the last fill.
      */
      static void GetFillInfo(const FIX42::ExecutionReport& message,
        Beam::Out<Quantity> lastQuantity, Beam::Out<Money> lastPrice);

      //! Constructs a FixOrderLog.
      FixOrderLog() = default;

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
        \param orderInfo The details of the Order to submit.
        \param senderCompId The session's SenderCompID.
        \param targetCompId The session's TargetCompID.
        \param f Receives the NewOrderSingle message to be submitted.
        \return The Order that was submitted.
      */
      template<typename F>
      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& orderInfo,
        const FIX::SenderCompID& senderCompId,
        const FIX::TargetCompID targetCompId, F f);

      //! Submits a OrderCancelRequest message.
      /*!
        \param session The session canceling the Order.
        \param orderId The Order id to cancel.
        \param timestamp The timestamp of the cancel request.
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
        \param timestamp The updates timestamp.
        \param f Receives the ExecutionReport that will be used to update the
                 Order.
      */
      template<typename F>
      void Update(const FIX42::ExecutionReport& message,
        const FIX::SessionID& sessionId,
        const boost::posix_time::ptime& timestamp, F f);

      //! Updates an Order based on an ExecutionReport.
      /*!
        \param sessionId The session's id.
        \param orderId The id of the Order to update.
        \param executionReport The ExecutionReport containing the update.
        \param timestamp The current timestamp.
      */
      void Update(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport,
        const boost::posix_time::ptime& timestamp);

    private:
      struct RecoveredExecutionReport {
        FIX42::ExecutionReport m_message;
        std::function<void (const OrderExecutionService::Order&,
          Beam::Out<OrderExecutionService::ExecutionReport>)> m_callback;

        RecoveredExecutionReport(const FIX42::ExecutionReport& message,
          const std::function<
          void (const OrderExecutionService::Order&,
          Beam::Out<OrderExecutionService::ExecutionReport>)>& callback);
      };
      Beam::Threading::Sync<std::unordered_map<OrderExecutionService::OrderId,
        std::shared_ptr<OrderExecutionService::PrimitiveOrder>>> m_orders;
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId,
        Beam::SynchronizedVector<RecoveredExecutionReport>>
        m_recoveredExecutionReports;

      std::shared_ptr<OrderExecutionService::PrimitiveOrder> AddOrder(
        const OrderExecutionService::OrderInfo& orderInfo, FIX::Side side);
      std::shared_ptr<OrderExecutionService::PrimitiveOrder> AddOrder(
        const OrderExecutionService::OrderRecord& orderRecord, FIX::Side side);
      std::shared_ptr<OrderExecutionService::PrimitiveOrder> RejectOrder(
        const OrderExecutionService::OrderInfo& orderInfo,
        const std::string& reason);
      template<typename F>
      void Update(const FIX42::ExecutionReport& message,
        const boost::posix_time::ptime& timestamp,
        const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
        F f);
  };

  inline FixOrderLog::RecoveredExecutionReport::RecoveredExecutionReport(
      const FIX42::ExecutionReport& message, const std::function<
      void (const OrderExecutionService::Order&,
      Beam::Out<OrderExecutionService::ExecutionReport>)>& callback)
      : m_message(message),
        m_callback(callback) {}

  inline boost::optional<OrderExecutionService::OrderId> FixOrderLog::
      GetOrderId(const FIX42::ExecutionReport& message) {
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
      return boost::none;
    }
    return orderId;
  }

  inline void FixOrderLog::GetFillInfo(const FIX42::ExecutionReport& message,
      Beam::Out<Quantity> lastQuantity, Beam::Out<Money> lastPrice) {
    if(!message.isSetField(FIX::FIELD::LastShares) ||
        !message.isSetField(FIX::FIELD::LastPx)) {
      *lastQuantity = 0;
      *lastPrice = Money::ZERO;
      return;
    }
    FIX::LastShares lastSharesField;
    message.get(lastSharesField);
    try {
      *lastQuantity = boost::lexical_cast<Quantity>(lastSharesField.getValue());
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
      FixOrderLog::FindOrder(OrderExecutionService::OrderId id) const {
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

  inline const OrderExecutionService::Order& FixOrderLog::Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    if((*orderRecord)->m_executionReports.empty()) {
      auto initialExecutionReport =
        OrderExecutionService::ExecutionReport::BuildInitialReport(
        (*orderRecord)->m_info.m_orderId, (*orderRecord)->m_info.m_timestamp);
      auto properOrderRecord = orderRecord;
      (*properOrderRecord)->m_executionReports.push_back(
        initialExecutionReport);
      return Recover(properOrderRecord);
    }
    auto side = GetSide((*orderRecord)->m_info.m_fields.m_side,
      (*orderRecord)->m_info.m_shortingFlag);
    if(!side.is_initialized()) {
      BOOST_THROW_EXCEPTION(
        OrderExecutionService::OrderUnrecoverableException());
    }
    auto order = AddOrder(**orderRecord, *side);
    auto recoveredExecutionReports = m_recoveredExecutionReports.Find(
      (*orderRecord)->m_info.m_orderId);
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
  const OrderExecutionService::Order& FixOrderLog::Submit(
      const OrderExecutionService::OrderInfo& orderInfo,
      const FIX::SenderCompID& senderCompId,
      const FIX::TargetCompID targetCompId, F f) {
    auto ordType = GetOrderType(orderInfo.m_fields.m_type);
    if(!ordType.is_initialized()) {
      return *RejectOrder(orderInfo, "Invalid order type.");
    }
    auto side = GetSide(orderInfo.m_fields.m_side, orderInfo.m_shortingFlag);
    if(!side.is_initialized()) {
      return *RejectOrder(orderInfo, "Invalid side.");
    }
    FIX42::NewOrderSingle newOrderSingle;
    FIX::ClOrdID clOrdId(boost::lexical_cast<std::string>(orderInfo.m_orderId));
    newOrderSingle.set(FIX::HandlInst('1'));
    newOrderSingle.set(clOrdId);
    FIX::Symbol symbol(orderInfo.m_fields.m_security.GetSymbol());
    newOrderSingle.set(symbol);
    newOrderSingle.set(*side);
    newOrderSingle.set(FIX::TransactTime(
      GetUtcTimestamp(orderInfo.m_timestamp)));
    newOrderSingle.set(*ordType);
    newOrderSingle.set(FIX::OrderQty(
      static_cast<FIX::QTY>(orderInfo.m_fields.m_quantity)));
    auto timeInForceType = GetTimeInForceType(
      orderInfo.m_fields.m_timeInForce.GetType());
    if(!timeInForceType.is_initialized()) {
      return *RejectOrder(orderInfo, "Invalid time in force.");
    }
    newOrderSingle.set(*timeInForceType);
    if(orderInfo.m_fields.m_type == OrderType::LIMIT ||
        (orderInfo.m_fields.m_type == OrderType::PEGGED &&
        orderInfo.m_fields.m_price != Money::ZERO)) {
      newOrderSingle.set(FIX::Price(ToDouble(orderInfo.m_fields.m_price)));
    }
    AddAdditionalTags(orderInfo.m_fields.m_additionalFields,
      Beam::Store(newOrderSingle));
    try {
      f(Beam::Store(newOrderSingle));
    } catch(const std::exception& e) {
      return *RejectOrder(orderInfo, e.what());
    }
    auto order = AddOrder(orderInfo, *side);
    FIX::Session::sendToTarget(newOrderSingle, senderCompId, targetCompId);
    return *order;
  }

  template<typename F>
  void FixOrderLog::Cancel(
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
    FIX42::OrderCancelRequest orderCancelRequest(origClOrdID, clOrdId,
      order->GetSymbol(), order->GetSide(), FIX::TransactTime());
    FIX::OrderQty orderQty(
      static_cast<FIX::QTY>(order->GetInfo().m_fields.m_quantity));
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
  void FixOrderLog::Update(const FIX42::ExecutionReport& message,
      const FIX::SessionID& sessionId,
      const boost::posix_time::ptime& timestamp, F f) {
    auto orderId = GetOrderId(message);
    if(!orderId.is_initialized()) {
      return;
    }

    // TODO: Turn this into a transaction to avoid possible race condition.
    auto order = FindOrder(*orderId);
    if(order == nullptr) {
      RecoveredExecutionReport recoveredExecutionReport(message, std::move(f));
      m_recoveredExecutionReports.Get(*orderId).PushBack(
        recoveredExecutionReport);
      return;
    }
    Update(message, timestamp, order, std::move(f));
  }

  inline std::shared_ptr<OrderExecutionService::PrimitiveOrder>
      FixOrderLog::AddOrder(const OrderExecutionService::OrderInfo& info,
      FIX::Side side) {
    auto order = std::make_shared<FixOrder>(info, side);
    Beam::Threading::With(m_orders,
      [&] (std::unordered_map<OrderExecutionService::OrderId,
          std::shared_ptr<OrderExecutionService::PrimitiveOrder>>& orders) {
        orders.insert(std::make_pair(info.m_orderId, order));
      });
    return order;
  }

  inline std::shared_ptr<OrderExecutionService::PrimitiveOrder>
      FixOrderLog::AddOrder(
      const OrderExecutionService::OrderRecord& orderRecord, FIX::Side side) {
    auto order = std::make_shared<FixOrder>(orderRecord, side);
    Beam::Threading::With(m_orders,
      [&] (std::unordered_map<OrderExecutionService::OrderId,
          std::shared_ptr<OrderExecutionService::PrimitiveOrder>>& orders) {
        orders.insert(std::make_pair(orderRecord.m_info.m_orderId, order));
      });
    return order;
  }

  inline std::shared_ptr<OrderExecutionService::PrimitiveOrder>
      FixOrderLog::RejectOrder(const OrderExecutionService::OrderInfo& info,
      const std::string& reason) {
    std::shared_ptr<OrderExecutionService::PrimitiveOrder> order =
      BuildRejectedOrder(info, reason);
    Beam::Threading::With(m_orders,
      [&] (std::unordered_map<OrderExecutionService::OrderId,
          std::shared_ptr<OrderExecutionService::PrimitiveOrder>>& orders) {
        orders.insert(std::make_pair(info.m_orderId, order));
      });
    return order;
  }

  template<typename F>
  void FixOrderLog::Update(const FIX42::ExecutionReport& message,
      const boost::posix_time::ptime& timestamp,
      const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
      F f) {
    FIX::ExecTransType execTransType;
    message.get(execTransType);
    if(execTransType != FIX::ExecTransType_NEW) {
      std::cout << "Trade Revised: " << message.toString() << std::endl;
      return;
    }
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
          if(filledQuantity >= order->GetInfo().m_fields.m_quantity) {
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

  inline void FixOrderLog::Update(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::ExecutionReport& executionReport,
      const boost::posix_time::ptime& timestamp) {
    auto order = std::dynamic_pointer_cast<FixOrder>(FindOrder(orderId));
    if(order == nullptr) {
      return;
    }
    auto updatedTimestamp =
      [&] {
        if(executionReport.m_timestamp.is_special()) {
          return timestamp;
        } else {
          return executionReport.m_timestamp;
        }
      }();
    order->With(
      [&] (auto status, const auto& executionReports) {
        if(IsTerminal(status) || executionReports.empty()) {
          return;
        }
        auto updatedExecutionReport = executionReport;
        updatedExecutionReport.m_sequence =
          executionReports.back().m_sequence + 1;
        updatedExecutionReport.m_timestamp = updatedTimestamp;
        if(executionReport.m_lastQuantity != 0) {
          Quantity filledQuantity = 0;
          for(auto& executionReport : executionReports) {
            filledQuantity += executionReport.m_lastQuantity;
          }
          updatedExecutionReport.m_lastQuantity =
            std::max<Quantity>(0, std::min(executionReport.m_lastQuantity,
            order->GetInfo().m_fields.m_quantity - filledQuantity));
          if(filledQuantity + updatedExecutionReport.m_lastQuantity >=
              order->GetInfo().m_fields.m_quantity) {
            updatedExecutionReport.m_status = OrderStatus::FILLED;
          } else {
            updatedExecutionReport.m_status = OrderStatus::PARTIALLY_FILLED;
          }
        }
        order->Update(updatedExecutionReport);
      });
  }
}
}

#endif
