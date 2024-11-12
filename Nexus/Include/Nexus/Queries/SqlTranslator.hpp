#ifndef NEXUS_SQL_EXPRESSION_TRANSLATOR_HPP
#define NEXUS_SQL_EXPRESSION_TRANSLATOR_HPP
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
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/Queries.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus::Queries {

  /** Translates an Expression into an SQL query. */
  class SqlTranslator : public Beam::Queries::SqlTranslator,
      public ExpressionVisitor {
    public:

      /**
       * Constructs an SqlTranslator.
       * @param parameter The parameter/table name.
       * @param expression The Expression to translate.
       */
      SqlTranslator(std::string parameter,
        Beam::Queries::Expression expression);

      void Visit(const Beam::Queries::ConstantExpression& expression) override;

      void Visit(
        const Beam::Queries::MemberAccessExpression& expression) override;

    private:
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

  /**
   * Translates an Expression into an SQL query.
   * @param parameter The parameter/table name.
   * @param expression The Expression to translate.
   * @return The SQL query representing the <i>expression</i>.
   */
  inline auto MakeSqlQuery(std::string parameter,
      Beam::Queries::Expression expression) {
    return Beam::Queries::MakeSqlQuery<SqlTranslator>(std::move(parameter),
      std::move(expression));
  }

  inline SqlTranslator::SqlTranslator(std::string parameter,
    Beam::Queries::Expression expression)
    : Beam::Queries::SqlTranslator(std::move(parameter),
        std::move(expression)) {}

  inline void SqlTranslator::Visit(
      const Beam::Queries::ConstantExpression& expression) {
    auto& value = expression.GetValue();
    if(value->GetType()->GetNativeType() == typeid(Quantity)) {
      GetTranslation() = Viper::literal(value->GetValue<Quantity>());
    } else if(value->GetType()->GetNativeType() == typeid(Money)) {
      GetTranslation() = Viper::literal(value->GetValue<Money>());
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::Visit(
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
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateSecurityMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetName() == "symbol" || expression.GetName() == "market" ||
        expression.GetName() == "country") {
      expression.GetExpression()->Apply(*this);
      auto term = GetTranslation();
      GetTranslation() = Viper::access(term, expression.GetName());
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateSecurityInfoMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetName() == "security") {
      expression.GetExpression()->Apply(*this);
      GetTranslation() = Viper::sym("");
    } else if(expression.GetName() == "name" ||
        expression.GetName() == "sector" ||
        expression.GetName() == "board_lot") {
      expression.GetExpression()->Apply(*this);
      auto term = GetTranslation();
      GetTranslation() = Viper::access(term, expression.GetName());
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateTimeAndSaleMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetName() == "timestamp" || expression.GetName() == "price" ||
        expression.GetName() == "size") {
      expression.GetExpression()->Apply(*this);
      auto term = GetTranslation();
      GetTranslation() = Viper::access(term, expression.GetName());
    } else if(expression.GetName() == "market_center") {
      expression.GetExpression()->Apply(*this);
      auto term = GetTranslation();
      GetTranslation() = Viper::access(term, "market");
    } else if(expression.GetName() == "buyer_mpid") {
      expression.GetExpression()->Apply(*this);
      auto term = GetTranslation();
      GetTranslation() = Viper::access(term, "buyer_mpid");
    } else if(expression.GetName() == "seller_mpid") {
      expression.GetExpression()->Apply(*this);
      auto term = GetTranslation();
      GetTranslation() = Viper::access(term, "seller_mpid");
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateOrderFieldsMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetName() == "security") {
      expression.GetExpression()->Apply(*this);
      GetTranslation() = Viper::sym("");
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }

  inline void SqlTranslator::TranslateOrderInfoMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetName() == "fields") {
      expression.GetExpression()->Apply(*this);
      GetTranslation() = Viper::sym("");
    } else if(expression.GetName() == "order_id" ||
        expression.GetName() == "shorting_flag" ||
        expression.GetName() == "timestamp" ||
        expression.GetName() == "is_live") {
      expression.GetExpression()->Apply(*this);
      auto term = GetTranslation();
      GetTranslation() = Viper::access(term, expression.GetName());
    } else {
      Beam::Queries::SqlTranslator::Visit(expression);
    }
  }
}

#endif
