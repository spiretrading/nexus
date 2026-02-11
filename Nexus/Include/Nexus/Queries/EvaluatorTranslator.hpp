#ifndef NEXUS_EVALUATOR_TRANSLATOR_HPP
#define NEXUS_EVALUATOR_TRANSLATOR_HPP
#include <memory>
#include <vector>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <Beam/Queries/FunctionEvaluatorNode.hpp>
#include <Beam/Queries/MemberAccessEvaluatorNode.hpp>
#include <Beam/Utilities/Casts.hpp>
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {

  /** Translates an Expression into an EvaluatorNode. */
  class EvaluatorTranslator : public Beam::EvaluatorTranslator<QueryTypes>,
      protected ExpressionVisitor {
    public:

      /** Constructs an EvaluatorTranslator. */
      EvaluatorTranslator();

      /**
       * Constructs an EvaluatorTranslator maintaining a set of live Orders.
       * @param live_orders The set of live Orders.
       */
      explicit EvaluatorTranslator(
        Beam::Ref<const Beam::SynchronizedUnorderedSet<OrderId>> live_orders);

      std::unique_ptr<Beam::EvaluatorTranslator<QueryTypes>>
        make_translator() const override;

    protected:
      void visit(const Beam::MemberAccessExpression& expression) override;

    private:
      const Beam::SynchronizedUnorderedSet<OrderId>* m_live_orders;

      void translate_ticker_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_ticker_info_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_time_and_sale_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_order_fields_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_order_info_member_access_expression(
        const Beam::MemberAccessExpression& expression);
  };

  inline EvaluatorTranslator::EvaluatorTranslator()
    : m_live_orders(nullptr) {}

  inline EvaluatorTranslator::EvaluatorTranslator(
    Beam::Ref<const Beam::SynchronizedUnorderedSet<OrderId>> live_orders)
    : m_live_orders(live_orders.get()) {}

  inline std::unique_ptr<Beam::EvaluatorTranslator<QueryTypes>>
      EvaluatorTranslator::make_translator() const {
    if(m_live_orders) {
      return std::make_unique<EvaluatorTranslator>(Beam::Ref(*m_live_orders));
    } else {
      return std::make_unique<EvaluatorTranslator>();
    }
  }

  inline void EvaluatorTranslator::visit(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_expression().get_type() == typeid(Ticker)) {
      translate_ticker_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(TickerInfo)) {
      translate_ticker_info_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(TimeAndSale)) {
      translate_time_and_sale_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(OrderFields)) {
      translate_order_fields_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(OrderInfo)) {
      translate_order_info_member_access_expression(expression);
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::translate_ticker_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    expression.get_expression().apply(*this);
    auto ticker_expression =
      Beam::static_pointer_cast<Beam::EvaluatorNode<Ticker>>(get_evaluator());
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
    expression.get_expression().apply(*this);
    auto ticker_info_expression = Beam::static_pointer_cast<
      Beam::EvaluatorNode<TickerInfo>>(get_evaluator());
    if(expression.get_name() == "name") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TickerInfo, std::string>>(
          std::move(ticker_info_expression), &TickerInfo::m_name));
    } else if(expression.get_name() == "instrument") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<TickerInfo, Instrument>>(
          std::move(ticker_info_expression), &TickerInfo::m_instrument));
    } else if(expression.get_name() == "tick_size") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<TickerInfo, Money>>(
          std::move(ticker_info_expression), &TickerInfo::m_tick_size));
    } else if(expression.get_name() == "lot_size") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<TickerInfo, Quantity>>(
          std::move(ticker_info_expression), &TickerInfo::m_lot_size));
    } else if(expression.get_name() == "board_lot") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<TickerInfo, Quantity>>(
          std::move(ticker_info_expression), &TickerInfo::m_board_lot));
    } else if(expression.get_name() == "price_precision") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<TickerInfo, int>>(
          std::move(ticker_info_expression), &TickerInfo::m_price_precision));
    } else if(expression.get_name() == "quantity_precision") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<TickerInfo, int>>(
          std::move(ticker_info_expression),
          &TickerInfo::m_quantity_precision));
    } else if(expression.get_name() == "multiplier") {
      set_evaluator(
        std::make_unique<Beam::MemberAccessEvaluatorNode<TickerInfo, Quantity>>(
          std::move(ticker_info_expression), &TickerInfo::m_multiplier));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_time_and_sale_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    expression.get_expression().apply(*this);
    auto time_and_sale_expression = Beam::static_pointer_cast<
      Beam::EvaluatorNode<TimeAndSale>>(get_evaluator());
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
    expression.get_expression().apply(*this);
    auto order_fields_expression = Beam::static_pointer_cast<
      Beam::EvaluatorNode<OrderFields>>(get_evaluator());
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
    expression.get_expression().apply(*this);
    auto order_info_expression = Beam::static_pointer_cast<
      Beam::EvaluatorNode<OrderInfo>>(get_evaluator());
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
}

#endif
