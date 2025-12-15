#ifndef NEXUS_FIX_ORDER_LOG_HPP
#define NEXUS_FIX_ORDER_LOG_HPP
#include <string>
#include <type_traits>
#include <variant>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Threading/Sync.hpp>
#include <boost/callable_traits/args.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include <quickfix/fix42/ExecutionReport.h>
#include <quickfix/fix42/NewOrderSingle.h>
#include <quickfix/fix42/OrderCancelRequest.h>
#include <quickfix/fix44/ExecutionReport.h>
#include <quickfix/fix44/NewOrderSingle.h>
#include <quickfix/fix44/OrderCancelRequest.h>
#include <quickfix/Session.h>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/FixUtilities/FixConversions.hpp"
#include "Nexus/FixUtilities/FixOrder.hpp"
#include "Nexus/FixUtilities/FixOrderRejectedException.hpp"

namespace Nexus {
namespace Details {
  template<typename T>
  struct ExtractOutParameter;

  template<typename T>
  struct ExtractOutParameter<Beam::Out<T>> {
    using type = T;
  };

  template<typename F>
  struct FixNewOrderSingle {
    using type = typename ExtractOutParameter<std::tuple_element_t<
      0, boost::callable_traits::args_t<std::remove_cvref_t<F>>>>::type;
  };

  template<typename F>
  struct FixCancelOrderRequest {
    using type = typename ExtractOutParameter<std::tuple_element_t<
      1, boost::callable_traits::args_t<std::remove_cvref_t<F>>>>::type;
  };
}

  /** Used to create and update FIX Orders. */
  class FixOrderLog {
    public:

      /**
       * Returns an Order's id from a FIX ExecutionReport message.
       * @param message The message to extract the Order id from.
       * @return The Order id represented by the <i>message</i>.
       */
      template<typename E>
      static boost::optional<OrderId> get_order_id(const E& message);

      /**
       * Returns information about a trade/fill from a FIX ExecutionReport
       * message.
       * @param message The message to extract the fill info from.
       * @param last_quantity Stores the quantity of the last fill.
       * @param last_price Stores the price of the last fill.
       */
      template<typename E>
      static void get_fill_info(const E& message,
        Beam::Out<Quantity> last_quantity, Beam::Out<Money> last_price);

      /** Constructs a FixOrderLog. */
      FixOrderLog() = default;

      /**
       * Returns the Order with a specified id.
       * @param id The Order id to find.
       * @return The Order with the specified <i>id</i> or
       *         <code>null</code> if no such Order exists.
       */
      std::shared_ptr<PrimitiveOrder> find(OrderId id) const;

      /**
       * Recovers a previously unlogged Order.
       * @param record The OrderRecord to recover.
       * @return The recovered Order.
       */
      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);

      /**
       * Submits a NewOrderSingle message.
       * @param info The details of the Order to submit.
       * @param sender_comp_id The session's SenderCompID.
       * @param target_comp_id The session's TargetCompID.
       * @param f Receives the NewOrderSingle message to be submitted.
       * @return The Order that was submitted.
       */
      template<typename F>
      std::shared_ptr<Order> submit(const OrderInfo& info,
        const FIX::SenderCompID& sender_comp_id,
        const FIX::TargetCompID target_comp_id, F&& f);

      /**
       * Submits a OrderCancelRequest message.
       * @param session The session canceling the Order.
       * @param id The Order id to cancel.
       * @param timestamp The timestamp of the cancel request.
       * @param sender_comp_id The session's SenderCompID.
       * @param target_comp_id The session's TargetCompID.
       * @param f Receives the OrderCancelRequest message to be submitted.
       */
      template<typename F>
      void cancel(const OrderExecutionSession& session,
        OrderId id, boost::posix_time::ptime timestamp,
        const FIX::SenderCompID& sender_comp_id,
        const FIX::TargetCompID target_comp_id, F&& f);

      /**
       * Updates an Order based on a FIX ExecutionReport message.
       * @param message The ExecutionReport updating the order.
       * @param session_id The session's id.
       * @param timestamp The updates timestamp.
       * @param f Receives the ExecutionReport that will be used to update the
       *          Order.
       */
      template<typename E, typename F>
      void update(const E& message, const FIX::SessionID& session_id,
        boost::posix_time::ptime timestamp, F&& f);

      /**
       * Updates an Order based on an ExecutionReport.
       * @param session_id The session's id.
       * @param id The id of the Order to update.
       * @param report The ExecutionReport containing the update.
       * @param timestamp The current timestamp.
       */
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report, boost::posix_time::ptime timestamp);

    private:
      using FixExecutionReport =
        std::variant<FIX42::ExecutionReport, FIX44::ExecutionReport>;
      struct RecoveredExecutionReport {
        FixExecutionReport m_message;
        std::function<void (
          const std::shared_ptr<Order>&, Beam::Out<ExecutionReport>)>
            m_callback;

        template<typename F>
        RecoveredExecutionReport(FixExecutionReport message, F&& callback);
      };
      Beam::Sync<
        std::unordered_map<OrderId, std::shared_ptr<PrimitiveOrder>>> m_orders;
      Beam::SynchronizedUnorderedMap<OrderId,
        Beam::SynchronizedVector<RecoveredExecutionReport>>
          m_recovered_reports;

      FixOrderLog(const FixOrderLog&) = delete;
      FixOrderLog& operator =(const FixOrderLog&) = delete;
      std::shared_ptr<PrimitiveOrder> add(
        const OrderInfo& info, FIX::Side side);
      std::shared_ptr<PrimitiveOrder> add(
        const OrderRecord& record, FIX::Side side);
      std::shared_ptr<PrimitiveOrder> reject(
        const OrderInfo& info, const std::string& reason);
      template<typename E, typename F>
      void update(const E& message, boost::posix_time::ptime timestamp,
        const std::shared_ptr<PrimitiveOrder>& order, F&& f);
  };

  template<typename F>
  FixOrderLog::RecoveredExecutionReport::RecoveredExecutionReport(
    FixExecutionReport message, F&& callback)
    : m_message(std::move(message)),
      m_callback(std::forward<F>(callback)) {}

  template<typename E>
  boost::optional<OrderId> FixOrderLog::get_order_id(const E& message) {
    auto client_order_id = FIX::ClOrdID();
    message.get(client_order_id);
    auto base_client_order_id = client_order_id.getString();
    auto delimiter = base_client_order_id.find('-');
    try {
      if(delimiter == std::string::npos) {
        return boost::lexical_cast<OrderId>(base_client_order_id);
      }
      return boost::lexical_cast<OrderId>(
        base_client_order_id.substr(0, delimiter));
    } catch(const boost::bad_lexical_cast&) {
      return boost::none;
    }
  }

  template<typename E>
  void FixOrderLog::get_fill_info(const E& message,
      Beam::Out<Quantity> last_quantity, Beam::Out<Money> last_price) {
    if(!message.isSetField(FIX::FIELD::LastShares) ||
        !message.isSetField(FIX::FIELD::LastPx)) {
      *last_quantity = 0;
      *last_price = Money::ZERO;
      return;
    }
    if constexpr(std::is_same_v<E, FIX42::ExecutionReport>) {
      auto last_shared_field = FIX::LastShares();
      message.get(last_shared_field);
      try {
        *last_quantity =
          boost::lexical_cast<Quantity>(last_shared_field.getValue());
      } catch(const boost::bad_lexical_cast&) {
        *last_quantity = 0;
        *last_price = Money::ZERO;
        return;
      }
    } else {
      auto last_quantity_field = FIX::LastQty();
      message.get(last_quantity_field);
      try {
        *last_quantity =
          boost::lexical_cast<Quantity>(last_quantity_field.getValue());
      } catch(const boost::bad_lexical_cast&) {
        *last_quantity = 0;
        *last_price = Money::ZERO;
        return;
      }
    }
    auto last_price_field = FIX::LastPx();
    message.get(last_price_field);
    auto last_price_field_value = try_parse_money(last_price_field.getString());
    if(last_price_field_value) {
      *last_price = *last_price_field_value;
    } else {
      *last_quantity = 0;
      *last_price = Money::ZERO;
    }
  }

  inline std::shared_ptr<PrimitiveOrder> FixOrderLog::find(OrderId id) const {
    return Beam::with(m_orders, [&] (const auto& orders) {
      auto i = orders.find(id);
      if(i == orders.end()) {
        return std::shared_ptr<PrimitiveOrder>();
      }
      return i->second;
    });
  }

  inline std::shared_ptr<Order> FixOrderLog::recover(
      const SequencedAccountOrderRecord& record) {
    if((*record)->m_execution_reports.empty()) {
      auto initial_report =
        ExecutionReport((*record)->m_info.m_id, (*record)->m_info.m_timestamp);
      auto proper_record = record;
      (*proper_record)->m_execution_reports.push_back(initial_report);
      return recover(proper_record);
    }
    auto side = get_side(
      (*record)->m_info.m_fields.m_side, (*record)->m_info.m_shorting_flag);
    if(!side) {
      boost::throw_with_location(OrderUnrecoverableException(
        "FIX order record missing a side: " + boost::lexical_cast<std::string>(
          (*record)->m_info.m_id)));
    }
    auto order = add(**record, *side);
    if(auto recovered_reports =
        m_recovered_reports.find((*record)->m_info.m_id)) {
      recovered_reports->for_each([&] (const auto& recovered_report) {
        auto fix_timestamp = FIX::UtcTimeStamp();
        std::visit([&] (auto& message) {
          if(message.isSetField(FIX::FIELD::TransactTime)) {
            auto transaction_time = FIX::TransactTime();
            message.get(transaction_time);
            fix_timestamp = static_cast<FIX::UtcTimeStamp>(transaction_time);
          } else {
            auto sending_time = FIX::SendingTime();
            message.getHeader().get(sending_time);
            fix_timestamp = static_cast<FIX::UtcTimeStamp>(sending_time);
          }
          auto timestamp = get_timestamp(fix_timestamp);
          update(message, timestamp, order, recovered_report.m_callback);
        }, recovered_report.m_message);
      });
    }
    return order;
  }

  template<typename F>
  std::shared_ptr<Order> FixOrderLog::submit(const OrderInfo& info,
      const FIX::SenderCompID& sender_comp_id,
      const FIX::TargetCompID target_comp_id, F&& f) {
    auto order_type = get_order_type(info.m_fields.m_type);
    if(!order_type) {
      return reject(info, "Invalid order type.");
    }
    auto side = get_side(info.m_fields.m_side, info.m_shorting_flag);
    if(!side) {
      return reject(info, "Invalid side.");
    }
    auto new_order_single = typename Details::FixNewOrderSingle<F>::type();
    auto client_order_id =
      FIX::ClOrdID(boost::lexical_cast<std::string>(info.m_id));
    new_order_single.set(FIX::HandlInst('1'));
    new_order_single.set(client_order_id);
    auto symbol = FIX::Symbol(info.m_fields.m_security.get_symbol());
    new_order_single.set(symbol);
    new_order_single.set(*side);
    new_order_single.set(
      FIX::TransactTime(get_utc_timestamp(info.m_timestamp)));
    new_order_single.set(*order_type);
    new_order_single.set(
      FIX::OrderQty(static_cast<FIX::QTY>(info.m_fields.m_quantity)));
    auto time_in_force_type =
      get_time_in_force_type(info.m_fields.m_time_in_force.get_type());
    if(!time_in_force_type) {
      return reject(info, "Invalid time in force.");
    }
    new_order_single.set(*time_in_force_type);
    if(info.m_fields.m_type == OrderType::LIMIT ||
        (info.m_fields.m_type == OrderType::PEGGED &&
          info.m_fields.m_price != Money::ZERO)) {
      new_order_single.set(
        FIX::Price(static_cast<double>(info.m_fields.m_price)));
    }
    add_additional_tags(
      info.m_fields.m_additional_fields, Beam::out(new_order_single));
    try {
      std::forward<F>(f)(Beam::out(new_order_single));
    } catch(const std::exception& e) {
      return reject(info, e.what());
    }
    auto order = add(info, *side);
    FIX::Session::sendToTarget(
      new_order_single, sender_comp_id, target_comp_id);
    return order;
  }

  template<typename F>
  void FixOrderLog::cancel(const OrderExecutionSession& session, OrderId id,
      boost::posix_time::ptime timestamp,
      const FIX::SenderCompID& sender_comp_id,
      const FIX::TargetCompID target_comp_id, F&& f) {
    auto order = std::dynamic_pointer_cast<FixOrder>(find(id));
    if(!order) {
      return;
    }
    auto original_client_order_id =
      FIX::OrigClOrdID(boost::lexical_cast<std::string>(id));
    auto client_order_id = order->get_next_cancel_id();
    auto order_cancel_request = [&] {
      using CancelRequest = typename Details::FixCancelOrderRequest<F>::type;
      if constexpr(std::is_same_v<CancelRequest, FIX42::OrderCancelRequest>) {
        return FIX42::OrderCancelRequest(original_client_order_id,
          client_order_id, order->get_symbol(), order->get_side(),
          FIX::TransactTime());
      } else {
        return FIX44::OrderCancelRequest(original_client_order_id,
          client_order_id, order->get_side(), FIX::TransactTime());
      }
    }();
    auto order_quantity = FIX::OrderQty(
      static_cast<FIX::QTY>(order->get_info().m_fields.m_quantity));
    order_cancel_request.set(order_quantity);
    std::forward<F>(f)(order, Beam::out(order_cancel_request));
    auto send_message = order->with([&] (auto status, const auto& reports) {
      if(is_terminal(status) || reports.empty()) {
        return false;
      }
      auto pending_cancel_report =
        make_update(reports.back(), OrderStatus::PENDING_CANCEL, timestamp);
      order->update(pending_cancel_report);
      return true;
    });
    if(send_message) {
      FIX::Session::sendToTarget(
        order_cancel_request, sender_comp_id, target_comp_id);
    }
  }

  template<typename E, typename F>
  void FixOrderLog::update(const E& message, const FIX::SessionID& session_id,
      boost::posix_time::ptime timestamp, F&& f) {
    auto id = get_order_id(message);
    if(!id) {
      return;
    }

    // TODO: Turn this into a transaction to avoid possible race condition.
    if(auto order = find(*id)) {
      update(message, timestamp, order, std::forward<F>(f));
    } else {
      auto recovered_report =
        RecoveredExecutionReport(message, std::forward<F>(f));
      m_recovered_reports.get(*id).push_back(recovered_report);
    }
  }

  inline std::shared_ptr<PrimitiveOrder> FixOrderLog::add(
      const OrderInfo& info, FIX::Side side) {
    auto order = std::make_shared<FixOrder>(info, side);
    Beam::with(m_orders, [&] (auto& orders) {
      orders.insert(std::pair(info.m_id, order));
    });
    return order;
  }

  inline std::shared_ptr<PrimitiveOrder> FixOrderLog::add(
      const OrderRecord& record, FIX::Side side) {
    auto order = std::make_shared<FixOrder>(record, side);
    Beam::with(m_orders, [&] (auto& orders) {
      orders.insert(std::pair(record.m_info.m_id, order));
    });
    return order;
  }

  inline std::shared_ptr<PrimitiveOrder> FixOrderLog::reject(
      const OrderInfo& info, const std::string& reason) {
    auto order = make_rejected_order(info, reason);
    Beam::with(m_orders, [&] (auto& orders) {
      orders.insert(std::pair(info.m_id, order));
    });
    return order;
  }

  template<typename E, typename F>
  void FixOrderLog::update(const E& message, boost::posix_time::ptime timestamp,
      const std::shared_ptr<PrimitiveOrder>& order, F&& f) {
    if constexpr(std::is_same_v<E, FIX42::ExecutionReport>) {
      auto exec_trans_type = FIX::ExecTransType();
      message.get(exec_trans_type);
      if(exec_trans_type != FIX::ExecTransType_NEW) {
        std::cout << "Trade Revised: " << message.toString() << std::endl;
        return;
      }
    }
    auto ord_status = FIX::OrdStatus();
    message.get(ord_status);
    auto order_status = get_order_status(ord_status);
    if(!order_status || *order_status == OrderStatus::PENDING_NEW) {
      return;
    }
    auto is_pending_cancel = *order_status == OrderStatus::PENDING_CANCEL;
    auto last_quantity = Quantity();
    auto last_price = Money();
    get_fill_info(message, Beam::out(last_quantity), Beam::out(last_price));
    if(is_pending_cancel && last_quantity == 0) {
      return;
    }
    auto text = FIX::Text();
    if(message.isSet(text)) {
      message.get(text);
    }
    order->with([&] (auto status, const auto& reports) {
      if(reports.empty() || is_terminal(reports.back().m_status)) {
        std::cout << "Stale Report: " << message.toString() << std::endl;
        return;
      }
      if(is_pending_cancel) {
        auto filled_quantity = last_quantity;
        for(auto& report : reports) {
          filled_quantity += report.m_last_quantity;
        }
        if(filled_quantity >= order->get_info().m_fields.m_quantity) {
          order_status = OrderStatus::FILLED;
        } else {
          order_status = OrderStatus::PARTIALLY_FILLED;
        }
      }
      auto updated_report =
        make_update(reports.back(), *order_status, timestamp);
      updated_report.m_last_quantity = last_quantity;
      updated_report.m_last_price = last_price;
      updated_report.m_text = text.getString();
      std::forward<F>(f)(order, Beam::out(updated_report));
      order->update(updated_report);
    });
  }

  inline void FixOrderLog::update(const OrderExecutionSession& session,
      OrderId id, const ExecutionReport& report,
      boost::posix_time::ptime timestamp) {
    auto order = std::dynamic_pointer_cast<FixOrder>(find(id));
    if(!order) {
      return;
    }
    auto updated_timestamp = [&] {
      if(report.m_timestamp.is_special()) {
        return timestamp;
      } else {
        return report.m_timestamp;
      }
    }();
    order->with([&] (auto status, const auto& reports) {
      if(is_terminal(status) || reports.empty()) {
        return;
      }
      auto updated_report = report;
      updated_report.m_sequence = reports.back().m_sequence + 1;
      updated_report.m_timestamp = updated_timestamp;
      if(report.m_last_quantity != 0) {
        auto filled_quantity = Quantity(0);
        for(auto& report : reports) {
          filled_quantity += report.m_last_quantity;
        }
        updated_report.m_last_quantity =
          std::max<Quantity>(0, std::min(report.m_last_quantity,
            order->get_info().m_fields.m_quantity - filled_quantity));
        if(filled_quantity + updated_report.m_last_quantity >=
            order->get_info().m_fields.m_quantity) {
          updated_report.m_status = OrderStatus::FILLED;
        } else {
          updated_report.m_status = OrderStatus::PARTIALLY_FILLED;
        }
      }
      order->update(updated_report);
    });
  }
}

#endif
