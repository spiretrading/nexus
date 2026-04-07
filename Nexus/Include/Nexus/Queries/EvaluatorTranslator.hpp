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

      void translate_security_member_access_expression(
        const Beam::MemberAccessExpression& expression);
      void translate_security_info_member_access_expression(
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
    if(expression.get_expression().get_type() == typeid(Security)) {
      translate_security_member_access_expression(expression);
    } else if(expression.get_expression().get_type() == typeid(SecurityInfo)) {
      translate_security_info_member_access_expression(expression);
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

  inline void EvaluatorTranslator::translate_security_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    expression.get_expression().apply(*this);
    auto security_expression =
      Beam::static_pointer_cast<Beam::EvaluatorNode<Security>>(get_evaluator());
    if(expression.get_name() == "symbol") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const Security& security) {
          return security.get_symbol();
        }, std::move(security_expression)));
    } else if(expression.get_name() == "venue") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const Security& security) {
          return static_cast<std::string>(
            security.get_venue().get_code().get_data());
        }, std::move(security_expression)));
    } else {
      Beam::EvaluatorTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_security_info_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    expression.get_expression().apply(*this);
    auto security_info_expression = Beam::static_pointer_cast<
      Beam::EvaluatorNode<SecurityInfo>>(get_evaluator());
    if(expression.get_name() == "security") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<SecurityInfo, Security>>(
          std::move(security_info_expression), &SecurityInfo::m_security));
    } else if(expression.get_name() == "name") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<SecurityInfo, std::string>>(
          std::move(security_info_expression), &SecurityInfo::m_name));
    } else if(expression.get_name() == "sector") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<SecurityInfo, std::string>>(
          std::move(security_info_expression), &SecurityInfo::m_sector));
    } else if(expression.get_name() == "board_lot") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<SecurityInfo, Quantity>>(
          std::move(security_info_expression), &SecurityInfo::m_board_lot));
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
    if(expression.get_name() == "security") {
      set_evaluator(std::make_unique<
        Beam::MemberAccessEvaluatorNode<OrderFields, Security>>(
          std::move(order_fields_expression), &OrderFields::m_security));
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
