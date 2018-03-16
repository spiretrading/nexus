#ifndef NEXUS_SQLEXPRESSIONTRANSLATOR_HPP
#define NEXUS_SQLEXPRESSIONTRANSLATOR_HPP
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/ExpressionVisitor.hpp>
#include <Beam/Queries/GlobalVariableDeclarationExpression.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/OrExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include <Beam/Queries/Queries.hpp>
#include <Beam/Queries/ReduceExpression.hpp>
#include <Beam/Queries/SetVariableExpression.hpp>
#include <Beam/Queries/SqlTranslator.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/Queries/VariableExpression.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/Queries.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {
namespace Queries {

  /*! \class SqlTranslator
      \brief Translates an Expression into an SQL query.
   */
  class SqlTranslator : public Beam::Queries::SqlTranslator,
      public ExpressionVisitor {
    public:

      //! Constructs an SqlTranslator.
      /*!
        \param parameter The parameter/table name.
        \param expression The Expression to translate.
      */
      SqlTranslator(std::string parameter,
        Beam::Queries::Expression expression);

      virtual void Visit(const Beam::Queries::ConstantExpression& expression)
        override;

      virtual void Visit(
        const Beam::Queries::MemberAccessExpression& expression) override;

    private:
      void TranslateSecurityMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateTimeAndSaleMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateOrderFieldsMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateOrderInfoMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
  };

  //! Translates an Expression into an SQL query.
  /*!
    \param parameter The parameter/table name.
    \param expression The Expression to translate.
    \return The SQL query representing the <i>expression</i>.
  */
  inline std::string BuildSqlQuery(std::string parameter,
      Beam::Queries::Expression expression) {
    SqlTranslator translator{std::move(parameter), std::move(expression)};
    return translator.BuildQuery();
  }

  inline SqlTranslator::SqlTranslator(std::string parameter,
      Beam::Queries::Expression expression)
      : Beam::Queries::SqlTranslator{std::move(parameter),
          std::move(expression)} {}

  inline void SqlTranslator::Visit(
      const Beam::Queries::ConstantExpression& expression) {
    auto& value = expression.GetValue();
    if(value->GetType()->GetNativeType() == typeid(Quantity)) {
      GetQuery() = boost::lexical_cast<std::string>(
        value->GetValue<Quantity>());
    } else if(value->GetType()->GetNativeType() == typeid(Money)) {
      GetQuery() = boost::lexical_cast<std::string>(value->GetValue<Money>());
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::Visit(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetExpression()->GetType() == SecurityType{}) {
      TranslateSecurityMemberAccessExpression(expression);
    } else if(expression.GetExpression()->GetType() == TimeAndSaleType{}) {
      TranslateTimeAndSaleMemberAccessExpression(expression);
    } else if(expression.GetExpression()->GetType() == OrderFieldsType{}) {
      TranslateOrderFieldsMemberAccessExpression(expression);
    } else if(expression.GetExpression()->GetType() == OrderInfoType{}) {
      TranslateOrderInfoMemberAccessExpression(expression);
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateSecurityMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(auto* parameter =
        dynamic_cast<const Beam::Queries::ParameterExpression*>(
        &*expression.GetExpression())) {
      GetQuery() = GetParameter() + ".";
    } else {
      auto& leftExpression = expression.GetExpression();
      leftExpression->Apply(*this);
    }
    if(expression.GetName() == "symbol") {
      GetQuery() += "symbol";
    } else if(expression.GetName() == "market") {
      GetQuery() += "market";
    } else if(expression.GetName() == "country") {
      GetQuery() += "country";
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateTimeAndSaleMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(auto* parameter =
        dynamic_cast<const Beam::Queries::ParameterExpression*>(
        &*expression.GetExpression())) {
      GetQuery() = GetParameter() + ".";
    }
    if(expression.GetName() == "timestamp") {
      GetQuery() += "timestamp";
    } else if(expression.GetName() == "price") {
      GetQuery() += "price";
    } else if(expression.GetName() == "size") {
      GetQuery() += "size";
    } else if(expression.GetName() == "market_center") {
      GetQuery() += "market";
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateOrderFieldsMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetName() == "security") {
      GetQuery().clear();
      return;
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateOrderInfoMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetName() == "fields") {
      GetQuery().clear();
      return;
    }
    if(auto* parameter =
        dynamic_cast<const Beam::Queries::ParameterExpression*>(
        &*expression.GetExpression())) {
      GetQuery() = GetParameter() + ".";
    }
    if(expression.GetName() == "order_id") {
      GetQuery() += "order_id";
    } else if(expression.GetName() == "shorting_flag") {
      GetQuery() += "shorting_flag";
    } else if(expression.GetName() == "timestamp") {
      GetQuery() += "timestamp";
    } else if(expression.GetName() == "is_live") {
      GetQuery() += "is_live";
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }
}
}

#endif
