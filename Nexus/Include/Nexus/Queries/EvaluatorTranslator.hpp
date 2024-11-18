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
       * @param liveOrders The set of live Orders.
       */
      EvaluatorTranslator(Beam::Ref<
        const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>>
          liveOrders);

      std::unique_ptr<Beam::Queries::EvaluatorTranslator<QueryTypes>>
        NewTranslator() const override;

    protected:
      void Visit(
        const Beam::Queries::MemberAccessExpression& expression) override;

    private:
      const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>*
        m_liveOrders;

      void TranslateSecurityMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateSecurityInfoMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateTimeAndSaleMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateOrderFieldsMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateOrderInfoMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
  };

  inline EvaluatorTranslator::EvaluatorTranslator()
    : m_liveOrders(nullptr) {}

  inline EvaluatorTranslator::EvaluatorTranslator(Beam::Ref<
    const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>>
      liveOrders)
    : m_liveOrders(liveOrders.Get()) {}

  inline std::unique_ptr<Beam::Queries::EvaluatorTranslator<QueryTypes>>
      EvaluatorTranslator::NewTranslator() const {
    if(m_liveOrders) {
      return std::make_unique<EvaluatorTranslator>(Beam::Ref(*m_liveOrders));
    } else {
      return std::make_unique<EvaluatorTranslator>();
    }
  }

  inline void EvaluatorTranslator::Visit(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetExpression()->GetType() == SecurityType()) {
      TranslateSecurityMemberAccessExpression(expression);
    } else if(expression.GetExpression()->GetType() == SecurityInfoType()) {
      TranslateSecurityInfoMemberAccessExpression(expression);
    } else if(expression.GetExpression()->GetType() == TimeAndSaleType()) {
      TranslateTimeAndSaleMemberAccessExpression(expression);
    } else if(expression.GetExpression()->GetType() == OrderFieldsType()) {
      TranslateOrderFieldsMemberAccessExpression(expression);
    } else if(expression.GetExpression()->GetType() == OrderInfoType()) {
      TranslateOrderInfoMemberAccessExpression(expression);
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateSecurityMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto securityExpression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<Security>>>(GetEvaluator());
    if(expression.GetName() == "symbol") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (const Security& security) {
          return security.GetSymbol();
        }, std::move(securityExpression)));
    } else if(expression.GetName() == "market") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (const Security& security) {
          return static_cast<std::string>(security.GetMarket().GetData());
        }, std::move(securityExpression)));
    } else if(expression.GetName() == "country") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (const Security& security) {
          return security.GetCountry();
        }, std::move(securityExpression)));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateSecurityInfoMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto securityInfoExpression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<SecurityInfo>>>(GetEvaluator());
    if(expression.GetName() == "security") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        Security, SecurityInfo>>(std::move(securityInfoExpression),
          &SecurityInfo::m_security));
    } else if(expression.GetName() == "name") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        std::string, SecurityInfo>>(std::move(securityInfoExpression),
          &SecurityInfo::m_name));
    } else if(expression.GetName() == "sector") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        std::string, SecurityInfo>>(std::move(securityInfoExpression),
          &SecurityInfo::m_sector));
    } else if(expression.GetName() == "board_lot") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        Quantity, SecurityInfo>>(std::move(securityInfoExpression),
          &SecurityInfo::m_boardLot));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateTimeAndSaleMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto timeAndSaleExpression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<TimeAndSale>>>(GetEvaluator());
    if(expression.GetName() == "timestamp") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<boost::posix_time::ptime,
        TimeAndSale>>(std::move(timeAndSaleExpression),
        &TimeAndSale::m_timestamp));
    } else if(expression.GetName() == "price") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Money, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_price));
    } else if(expression.GetName() == "size") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Quantity, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_size));
    } else if(expression.GetName() == "market_center") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_marketCenter));
    } else if(expression.GetName() == "buyer_mpid") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_buyerMpid));
    } else if(expression.GetName() == "seller_mpid") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_sellerMpid));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateOrderFieldsMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto orderFieldsExpression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<OrderExecutionService::OrderFields>>>(
      GetEvaluator());
    if(expression.GetName() == "security") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        Security, OrderExecutionService::OrderFields>>(
        std::move(orderFieldsExpression),
        &OrderExecutionService::OrderFields::m_security));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateOrderInfoMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto orderInfoExpression = Beam::StaticCast<std::unique_ptr<
      Beam::Queries::EvaluatorNode<OrderExecutionService::OrderInfo>>>(
      GetEvaluator());
    if(expression.GetName() == "fields") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        OrderExecutionService::OrderFields, OrderExecutionService::OrderInfo>>(
        std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_fields));
    } else if(expression.GetName() == "order_id") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        OrderExecutionService::OrderId, OrderExecutionService::OrderInfo>>(
        std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_orderId));
    } else if(expression.GetName() == "shorting_flag") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        bool, OrderExecutionService::OrderInfo>>(std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_shortingFlag));
    } else if(expression.GetName() == "timestamp") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        boost::posix_time::ptime, OrderExecutionService::OrderInfo>>(
        std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_timestamp));
    } else if(expression.GetName() == "is_live") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [liveOrders = m_liveOrders] (
            const OrderExecutionService::OrderInfo& orderInfo) {
          if(!liveOrders) {
            return false;
          }
          return liveOrders->Contains(orderInfo.m_orderId);
        }, std::move(orderInfoExpression)));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }
}

#endif
