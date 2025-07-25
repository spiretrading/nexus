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
#include <boost/throw_exception.hpp>
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/Queries.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus::Queries {

  /** Translates an Expression into an EvaluatorNode. */
  class EvaluatorTranslator :
      public Beam::Queries::EvaluatorTranslator<QueryTypes>,
      protected ExpressionVisitor {
    public:

      /** Constructs an EvaluatorTranslator. */
      EvaluatorTranslator();

      /**
       * Constructs an EvaluatorTranslator maintaining a set of live Orders.
       * @param live_orders The set of live Orders.
       */
      explicit EvaluatorTranslator(Beam::Ref<
        const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>>
          live_orders);

      std::unique_ptr<Beam::Queries::EvaluatorTranslator<QueryTypes>>
        NewTranslator() const override;

    protected:
      void Visit(
        const Beam::Queries::MemberAccessExpression& expression) override;

    private:
      const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>*
        m_live_orders;

      void translate_security_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression);
      void translate_security_info_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression);
      void translate_time_and_sale_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression);
      void translate_order_fields_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression);
      void translate_order_info_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression);
  };

  inline EvaluatorTranslator::EvaluatorTranslator()
    : m_live_orders(nullptr) {}

  inline EvaluatorTranslator::EvaluatorTranslator(Beam::Ref<
    const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>>
      live_orders)
    : m_live_orders(live_orders.Get()) {}

  inline std::unique_ptr<Beam::Queries::EvaluatorTranslator<QueryTypes>>
      EvaluatorTranslator::NewTranslator() const {
    if(m_live_orders) {
      return std::make_unique<EvaluatorTranslator>(Beam::Ref(*m_live_orders));
    } else {
      return std::make_unique<EvaluatorTranslator>();
    }
  }

  inline void EvaluatorTranslator::Visit(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetExpression()->GetType() == SecurityType()) {
      translate_security_member_access_expression(expression);
    } else if(expression.GetExpression()->GetType() == SecurityInfoType()) {
      translate_security_info_member_access_expression(expression);
    } else if(expression.GetExpression()->GetType() == TimeAndSaleType()) {
      translate_time_and_sale_member_access_expression(expression);
    } else if(expression.GetExpression()->GetType() == OrderFieldsType()) {
      translate_order_fields_member_access_expression(expression);
    } else if(expression.GetExpression()->GetType() == OrderInfoType()) {
      translate_order_info_member_access_expression(expression);
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::translate_security_member_access_expression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto security_expression = Beam::StaticCast<
      std::unique_ptr<Beam::Queries::EvaluatorNode<Security>>>(GetEvaluator());
    if(expression.GetName() == "symbol") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (const Security& security) {
          return security.get_symbol();
        }, std::move(security_expression)));
    } else if(expression.GetName() == "venue") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (const Security& security) {
          return static_cast<std::string>(
            security.get_venue().get_code().GetData());
        }, std::move(security_expression)));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_security_info_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto security_info_expression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<SecurityInfo>>>(GetEvaluator());
    if(expression.GetName() == "security") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Security, SecurityInfo>>(
          std::move(security_info_expression), &SecurityInfo::m_security));
    } else if(expression.GetName() == "name") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, SecurityInfo>>(
          std::move(security_info_expression), &SecurityInfo::m_name));
    } else if(expression.GetName() == "sector") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, SecurityInfo>>(
          std::move(security_info_expression), &SecurityInfo::m_sector));
    } else if(expression.GetName() == "board_lot") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Quantity, SecurityInfo>>(
          std::move(security_info_expression), &SecurityInfo::m_board_lot));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_time_and_sale_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto time_and_sale_expression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<TimeAndSale>>>(GetEvaluator());
    if(expression.GetName() == "timestamp") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        boost::posix_time::ptime, TimeAndSale>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_timestamp));
    } else if(expression.GetName() == "price") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Money, TimeAndSale>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_price));
    } else if(expression.GetName() == "size") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Quantity, TimeAndSale>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_size));
    } else if(expression.GetName() == "market_center") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, TimeAndSale>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_market_center));
    } else if(expression.GetName() == "buyer_mpid") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, TimeAndSale>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_buyer_mpid));
    } else if(expression.GetName() == "seller_mpid") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, TimeAndSale>>(
          std::move(time_and_sale_expression), &TimeAndSale::m_seller_mpid));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_order_fields_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto order_fields_expression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<OrderExecutionService::OrderFields>>>(
        GetEvaluator());
    if(expression.GetName() == "security") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        Security, OrderExecutionService::OrderFields>>(
          std::move(order_fields_expression),
          &OrderExecutionService::OrderFields::m_security));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::
      translate_order_info_member_access_expression(
        const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto order_info_expression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<OrderExecutionService::OrderInfo>>>(
        GetEvaluator());
    if(expression.GetName() == "fields") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        OrderExecutionService::OrderFields, OrderExecutionService::OrderInfo>>(
          std::move(order_info_expression),
          &OrderExecutionService::OrderInfo::m_fields));
    } else if(expression.GetName() == "order_id") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        OrderExecutionService::OrderId, OrderExecutionService::OrderInfo>>(
          std::move(order_info_expression),
          &OrderExecutionService::OrderInfo::m_id));
    } else if(expression.GetName() == "shorting_flag") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        bool, OrderExecutionService::OrderInfo>>(
          std::move(order_info_expression),
          &OrderExecutionService::OrderInfo::m_shorting_flag));
    } else if(expression.GetName() == "timestamp") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        boost::posix_time::ptime, OrderExecutionService::OrderInfo>>(
          std::move(order_info_expression),
          &OrderExecutionService::OrderInfo::m_timestamp));
    } else if(expression.GetName() == "is_live") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [live_orders = m_live_orders] (
            const OrderExecutionService::OrderInfo& info) {
          if(!live_orders) {
            return false;
          }
          return live_orders->Contains(info.m_id);
        }, std::move(order_info_expression)));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }
}

#endif
