#ifndef NEXUS_SQL_EXPRESSION_TRANSLATOR_HPP
#define NEXUS_SQL_EXPRESSION_TRANSLATOR_HPP
#include <string>
#include <boost/lexical_cast.hpp>
#include <Beam/Queries/ConstantExpression.hpp>
#include <Beam/Queries/ExpressionVisitor.hpp>
#include <Beam/Queries/GlobalVariableDeclarationExpression.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/OrExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include <Beam/Queries/ReduceExpression.hpp>
#include <Beam/Queries/SetVariableExpression.hpp>
#include <Beam/Queries/SqlTranslator.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include <Beam/Queries/VariableExpression.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/SqlDefinitions.hpp"
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {

  /** Translates an Expression into an SQL query. */
  class SqlTranslator : public Beam::SqlTranslator, public ExpressionVisitor {
    public:

      /**
       * Constructs an SqlTranslator.
       * @param parameter The parameter/table name.
       * @param expression The Expression to translate.
       */
      SqlTranslator(std::string parameter, Beam::Expression expression);

    protected:
      void visit(const Beam::ConstantExpression& expression) override;
      void visit(const Beam::MemberAccessExpression& expression) override;

    private:
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
  };

  /**
   * Translates an Expression into an SQL query.
   * @param parameter The parameter/table name.
   * @param expression The Expression to translate.
   * @return The SQL query representing the <i>expression</i>.
   */
  inline auto make_sql_query(
      std::string parameter, Beam::Expression expression) {
    return Beam::make_sql_query<SqlTranslator>(
      std::move(parameter), std::move(expression));
  }

  inline SqlTranslator::SqlTranslator(
    std::string parameter, Beam::Expression expression)
    : Beam::SqlTranslator(std::move(parameter), std::move(expression)) {}

  inline void SqlTranslator::visit(const Beam::ConstantExpression& expression) {
    auto& value = expression.get_value();
    if(value.get_type() == typeid(Quantity)) {
      get_translation() = Viper::literal(value.as<Quantity>());
    } else if(value.get_type() == typeid(Money)) {
      get_translation() = Viper::literal(value.as<Money>());
    } else if(value.get_type() == typeid(Side)) {
      get_translation() = Viper::literal(static_cast<int>(value.as<Side>()));
    } else if(value.get_type() == typeid(Venue)) {
      get_translation() = Viper::literal(value.as<Venue>());
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::visit(
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
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_ticker_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "symbol" || expression.get_name() == "venue") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_ticker_info_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "ticker") {
      expression.get_expression().apply(*this);
      get_translation() = Viper::sym("");
    } else if(expression.get_name() == "name" ||
        expression.get_name() == "sector" ||
        expression.get_name() == "board_lot") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "price" || expression.get_name() == "size" ||
        expression.get_name() == "side") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_bbo_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "bid" || expression.get_name() == "ask") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else if(expression.get_name() == "timestamp") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "timestamp");
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_book_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "mpid" || expression.get_name() == "timestamp") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else if(expression.get_name() == "is_primary_mpid") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "is_primary");
    } else if(expression.get_name() == "venue") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "quote_venue");
    } else if(expression.get_name() == "quote") {
      expression.get_expression().apply(*this);
      get_translation() = Viper::sym("");
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_order_imbalance_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "ticker") {
      expression.get_expression().apply(*this);
      get_translation() = Viper::sym("");
    } else if(expression.get_name() == "side" ||
        expression.get_name() == "size") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else if(expression.get_name() == "reference_price") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "price");
    } else if(expression.get_name() == "timestamp") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "timestamp");
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_ticker_status_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "state" || expression.get_name() == "flags" ||
        expression.get_name() == "timestamp") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else if(expression.get_name() == "venue") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "status_venue");
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_time_and_sale_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "timestamp" ||
        expression.get_name() == "price" || expression.get_name() == "size") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else if(expression.get_name() == "market_center") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "market");
    } else if(expression.get_name() == "buyer_mpid") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "buyer_mpid");
    } else if(expression.get_name() == "seller_mpid") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, "seller_mpid");
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_order_fields_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "ticker") {
      expression.get_expression().apply(*this);
      get_translation() = Viper::sym("");
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }

  inline void SqlTranslator::translate_order_info_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "fields") {
      expression.get_expression().apply(*this);
      get_translation() = Viper::sym("");
    } else if(expression.get_name() == "order_id" ||
        expression.get_name() == "shorting_flag" ||
        expression.get_name() == "timestamp" ||
        expression.get_name() == "is_live") {
      expression.get_expression().apply(*this);
      auto term = get_translation();
      get_translation() = Viper::access(term, expression.get_name());
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }
}

#endif
