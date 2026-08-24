#ifndef NEXUS_EVALUATOR_TRANSLATOR_HPP
#define NEXUS_EVALUATOR_TRANSLATOR_HPP
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <Beam/Queries/ExpressionTranslationException.hpp>
#include <Beam/Queries/FunctionEvaluatorNode.hpp>
#include <Beam/Queries/MemberAccessEvaluatorNode.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {

  /**
   * Stores the status updates of account modification requests, keyed by the
   * id of the request they belong to.
   */
  using AccountModificationRequestUpdates =
    std::unordered_map<AccountModificationRequest::Id,
      std::vector<AccountModificationRequest::Update>>;

  /** Translates an Expression into an EvaluatorNode. */
  class EvaluatorTranslator : public Beam::EvaluatorTranslator<QueryTypes>,
      protected ExpressionVisitor {
    public:

      /**
       * Constructs an EvaluatorTranslator for an Expression evaluated without
       * a parameter.
       */
      EvaluatorTranslator();

      /**
       * Constructs an EvaluatorTranslator.
       * @param type The type of value the parameters are bound to.
       */
      explicit EvaluatorTranslator(std::type_index type);

      /**
       * Constructs an EvaluatorTranslator maintaining a set of live Orders.
       * @param type The type of value the parameters are bound to.
       * @param live_orders The set of live Orders.
       */
      EvaluatorTranslator(std::type_index type,
        Beam::Ref<const Beam::SynchronizedUnorderedSet<OrderId>> live_orders);

      /**
       * Constructs an EvaluatorTranslator able to access the members of an
       * account modification request that are stored separately from it.
       * @param type The type of value the parameters are bound to.
       * @param request_updates The updates of every account modification
       *        request.
       */
      EvaluatorTranslator(std::type_index type,
        Beam::Ref<const AccountModificationRequestUpdates> request_updates);

      std::unique_ptr<Beam::EvaluatorTranslator<QueryTypes>>
        make_translator(std::type_index type) const override;

    protected:
      void visit(const Beam::MemberAccessExpression& expression) override;

    private:
      const Beam::SynchronizedUnorderedSet<OrderId>* m_live_orders;
      const AccountModificationRequestUpdates* m_request_updates;

      static const std::vector<AccountModificationRequest::Update>*
        find_updates(const AccountModificationRequestUpdates& updates,
          AccountModificationRequest::Id id);
      const AccountModificationRequestUpdates& require_request_updates() const;
      void translate_ticker_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_ticker_info_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_quote_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_bbo_quote_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_book_quote_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_order_imbalance_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_ticker_status_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_time_and_sale_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_order_fields_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_order_info_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_account_modification_request_member_access_expression(
        const Beam::MemberAccessExpression& expression);
  };

  inline EvaluatorTranslator::EvaluatorTranslator()
    : EvaluatorTranslator(typeid(void)) {}

  inline EvaluatorTranslator::EvaluatorTranslator(std::type_index type)
    : Beam::EvaluatorTranslator<QueryTypes>(type),
      m_live_orders(nullptr),
      m_request_updates(nullptr) {}

  inline EvaluatorTranslator::EvaluatorTranslator(std::type_index type,
    Beam::Ref<const Beam::SynchronizedUnorderedSet<OrderId>> live_orders)
    : Beam::EvaluatorTranslator<QueryTypes>(type),
      m_live_orders(live_orders.get()),
      m_request_updates(nullptr) {}

  inline EvaluatorTranslator::EvaluatorTranslator(std::type_index type,
    Beam::Ref<const AccountModificationRequestUpdates> request_updates)
    : Beam::EvaluatorTranslator<QueryTypes>(type),
      m_live_orders(nullptr),
      m_request_updates(request_updates.get()) {}

  inline std::unique_ptr<Beam::EvaluatorTranslator<QueryTypes>>
      EvaluatorTranslator::make_translator(std::type_index type) const {
    auto translator = std::make_unique<EvaluatorTranslator>(type);
    translator->m_live_orders = m_live_orders;
    translator->m_request_updates = m_request_updates;
    return translator;
  }

  inline const std::vector<AccountModificationRequest::Update>*
      EvaluatorTranslator::find_updates(
        const AccountModificationRequestUpdates& updates,
        AccountModificationRequest::Id id) {
    auto entry = updates.find(id);
    if(entry == updates.end() || entry->second.empty()) {
      return nullptr;
    }
    return &entry->second;
  }

  inline const AccountModificationRequestUpdates&
      EvaluatorTranslator::require_request_updates() const {
    if(!m_request_updates) {
      boost::throw_with_location(Beam::ExpressionTranslationException(
        "Account modification request updates not available."));
    }
    return *m_request_updates;
  }

  inline void EvaluatorTranslator::visit(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_expression().get_type() == typeid(Quote)) {
      translate_quote_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(BboQuote)) {
      translate_bbo_quote_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(BookQuote)) {
      translate_book_quote_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(Ticker)) {
      translate_ticker_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(TickerInfo)) {
      translate_ticker_info_member_access_expression(expression);
    } else if(
        expression.get_expression().get_type() == typeid(OrderImbalance)) {
      translate_order_imbalance_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(TickerStatus)) {
      translate_ticker_status_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(TimeAndSale)) {
      translate_time_and_sale_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(OrderFields)) {
      translate_order_fields_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(OrderInfo)) {
      translate_order_info_member_access_expression(expression);
    } else if(expression.get_expression().get_type() ==
        typeid(AccountModificationRequest)) {
      translate_account_modification_request_member_access_expression(
        expression);
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::translate_ticker_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    auto ticker_expression =
      translate_operand<Ticker>(expression.get_expression());
    if(expression.get_name() == "symbol") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const Ticker& ticker) {
          return ticker.get_symbol();
        }, std::move(ticker_expression)));
    } else if(expression.get_name() == "venue") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const Ticker& ticker) {
          return static_cast<std::string>(
            ticker.get_venue().get_code().get_data());
        }, std::move(ticker_expression)));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_ticker_info_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto ticker_info_expression =
      translate_operand<TickerInfo>(expression.get_expression());
    if(expression.get_name() == "ticker") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TickerInfo, Ticker>>(
          std::move(ticker_info_expression), &TickerInfo::m_ticker));
    } else if(expression.get_name() == "name") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TickerInfo, std::string>>(
          std::move(ticker_info_expression), &TickerInfo::m_name));
    } else if(expression.get_name() == "sector") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TickerInfo, std::string>>(
          std::move(ticker_info_expression), &TickerInfo::m_sector));
    } else if(expression.get_name() == "board_lot") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TickerInfo, Quantity>>(
          std::move(ticker_info_expression), &TickerInfo::m_board_lot));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::translate_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    auto quote_expression =
      translate_operand<Quote>(expression.get_expression());
    if(expression.get_name() == "price") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<Quote, Money>>(
          std::move(quote_expression), &Quote::m_price));
    } else if(expression.get_name() == "size") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<Quote, Quantity>>(
          std::move(quote_expression), &Quote::m_size));
    } else if(expression.get_name() == "side") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<Quote, Side>>(
          std::move(quote_expression), &Quote::m_side));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::translate_bbo_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    auto bbo_quote_expression =
      translate_operand<BboQuote>(expression.get_expression());
    if(expression.get_name() == "bid") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<BboQuote, Quote>>(
          std::move(bbo_quote_expression), &BboQuote::m_bid));
    } else if(expression.get_name() == "ask") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<BboQuote, Quote>>(
          std::move(bbo_quote_expression), &BboQuote::m_ask));
    } else if(expression.get_name() == "timestamp") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<BboQuote, boost::posix_time::ptime>>(
          std::move(bbo_quote_expression), &BboQuote::m_timestamp));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_book_quote_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto book_quote_expression =
      translate_operand<BookQuote>(expression.get_expression());
    if(expression.get_name() == "mpid") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<BookQuote, std::string>>(
          std::move(book_quote_expression), &BookQuote::m_mpid));
    } else if(expression.get_name() == "is_primary_mpid") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<BookQuote, bool>>(
          std::move(book_quote_expression), &BookQuote::m_is_primary_mpid));
    } else if(expression.get_name() == "venue") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<BookQuote, Venue>>(
          std::move(book_quote_expression), &BookQuote::m_venue));
    } else if(expression.get_name() == "quote") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<BookQuote, Quote>>(
          std::move(book_quote_expression), &BookQuote::m_quote));
    } else if(expression.get_name() == "timestamp") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<BookQuote, boost::posix_time::ptime>>(
          std::move(book_quote_expression), &BookQuote::m_timestamp));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_order_imbalance_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto imbalance_expression =
      translate_operand<OrderImbalance>(expression.get_expression());
    if(expression.get_name() == "ticker") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderImbalance, Ticker>>(
          std::move(imbalance_expression), &OrderImbalance::m_ticker));
    } else if(expression.get_name() == "side") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<OrderImbalance, Side>>(
          std::move(imbalance_expression), &OrderImbalance::m_side));
    } else if(expression.get_name() == "size") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderImbalance, Quantity>>(
          std::move(imbalance_expression), &OrderImbalance::m_size));
    } else if(expression.get_name() == "reference_price") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderImbalance, Money>>(
          std::move(imbalance_expression), &OrderImbalance::m_reference_price));
    } else if(expression.get_name() == "timestamp") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<OrderImbalance,
          boost::posix_time::ptime>>(
            std::move(imbalance_expression), &OrderImbalance::m_timestamp));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_ticker_status_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto status_expression =
      translate_operand<TickerStatus>(expression.get_expression());
    if(expression.get_name() == "venue") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<TickerStatus, Venue>>(
          std::move(status_expression), &TickerStatus::m_venue));
    } else if(expression.get_name() == "state") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TickerStatus, std::string>>(
          std::move(status_expression), &TickerStatus::m_state));
    } else if(expression.get_name() == "flags") {
      set_evaluator(
        Beam::make_function_evaluator_node([] (const TickerStatus& status) {
          return static_cast<int>(status.m_flags);
        }, std::move(status_expression)));
    } else if(expression.get_name() == "timestamp") {
      set_evaluator(std::make_unique<Beam::MemberAccessEvaluatorNode<
        TickerStatus, boost::posix_time::ptime>>(
          std::move(status_expression), &TickerStatus::m_timestamp));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_time_and_sale_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto time_and_sale_expression =
      translate_operand<TimeAndSale>(expression.get_expression());
    if(expression.get_name() == "timestamp") {
      set_evaluator(std::make_unique<Beam::MemberAccessEvaluatorNode<
        TimeAndSale, boost::posix_time::ptime>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_timestamp));
    } else if(expression.get_name() == "price") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TimeAndSale, Money>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_price));
    } else if(expression.get_name() == "size") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TimeAndSale, Quantity>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_size));
    } else if(expression.get_name() == "market_center") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TimeAndSale, std::string>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_market_center));
    } else if(expression.get_name() == "buyer_mpid") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TimeAndSale, std::string>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_buyer_mpid));
    } else if(expression.get_name() == "seller_mpid") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TimeAndSale, std::string>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_seller_mpid));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_order_fields_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto order_fields_expression =
      translate_operand<OrderFields>(expression.get_expression());
    if(expression.get_name() == "ticker") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderFields, Ticker>>(
          std::move(order_fields_expression), &OrderFields::m_ticker));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_order_info_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto order_info_expression =
      translate_operand<OrderInfo>(expression.get_expression());
    if(expression.get_name() == "fields") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderInfo, OrderFields>>(
          std::move(order_info_expression), &OrderInfo::m_fields));
    } else if(expression.get_name() == "order_id") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderInfo, OrderId>>(
          std::move(order_info_expression), &OrderInfo::m_id));
    } else if(expression.get_name() == "shorting_flag") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderInfo, bool>>(
          std::move(order_info_expression), &OrderInfo::m_shorting_flag));
    } else if(expression.get_name() == "timestamp") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderInfo, boost::posix_time::ptime>>(
          std::move(order_info_expression), &OrderInfo::m_timestamp));
    } else if(expression.get_name() == "is_live") {
      set_evaluator(Beam::make_function_evaluator_node(
        [live_orders = m_live_orders] (const OrderInfo& info) {
          if(!live_orders) {
            return false;
          }
          return live_orders->contains(info.m_id);
        }, std::move(order_info_expression)));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_account_modification_request_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    auto request_expression =
      translate_operand<AccountModificationRequest>(expression.get_expression());
    if(expression.get_name() == "id") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return request.get_id();
        }, std::move(request_expression)));
    } else if(expression.get_name() == "type") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return static_cast<int>(request.get_type());
        }, std::move(request_expression)));
    } else if(expression.get_name() == "account") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return static_cast<int>(request.get_account().m_id);
        }, std::move(request_expression)));
    } else if(expression.get_name() == "submission_account") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return static_cast<int>(request.get_submission_account().m_id);
        }, std::move(request_expression)));
    } else if(expression.get_name() == "timestamp") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return request.get_timestamp();
        }, std::move(request_expression)));
    } else if(expression.get_name() == "effective_date") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return request.get_effective_date();
        }, std::move(request_expression)));
    } else if(expression.get_name() == "status") {
      set_evaluator(Beam::make_function_evaluator_node(
        [&updates = require_request_updates()] (
            const AccountModificationRequest& request) {
          if(auto entry = find_updates(updates, request.get_id())) {
            return static_cast<int>(entry->back().m_status);
          }
          return static_cast<int>(AccountModificationRequest::Status::NONE);
        }, std::move(request_expression)));
    } else if(expression.get_name() == "last_update_timestamp") {
      set_evaluator(Beam::make_function_evaluator_node(
        [&updates = require_request_updates()] (
            const AccountModificationRequest& request) {
          if(auto entry = find_updates(updates, request.get_id())) {
            return entry->back().m_timestamp;
          }
          return request.get_timestamp();
        }, std::move(request_expression)));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }
}

#endif
