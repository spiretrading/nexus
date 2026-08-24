#ifndef NEXUS_SQL_EXPRESSION_TRANSLATOR_HPP
#define NEXUS_SQL_EXPRESSION_TRANSLATOR_HPP
#include <string>
#include <typeindex>
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
  class SqlTranslator :
      public Beam::SqlTranslator<QueryTypes>, public ExpressionVisitor {
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
      void translate_access(const Beam::MemberAccessExpression& expression,
        const std::string& column, std::type_index type);
      void translate_symbol(const Beam::MemberAccessExpression& expression,
        const std::string& column, std::type_index type);
      void translate_embedded(
        const Beam::MemberAccessExpression& expression, std::type_index type);
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
    : Beam::SqlTranslator<QueryTypes>(
        std::move(parameter), std::move(expression)) {}

  inline void SqlTranslator::visit(const Beam::ConstantExpression& expression) {
    auto& value = expression.get_value();
    if(value.get_type() == typeid(Quantity)) {
      set_translation(Viper::literal(value.as<Quantity>()), value.get_type());
    } else if(value.get_type() == typeid(Money)) {
      set_translation(Viper::literal(value.as<Money>()), value.get_type());
    } else if(value.get_type() == typeid(Side)) {
      set_translation(
        Viper::literal(static_cast<int>(value.as<Side>())), value.get_type());
    } else if(value.get_type() == typeid(Venue)) {
      set_translation(Viper::literal(value.as<Venue>()), value.get_type());
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
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
    } else if(expression.get_expression().get_type() ==
        typeid(AccountModificationRequest)) {
      translate_account_modification_request_member_access_expression(
        expression);
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_access(
      const Beam::MemberAccessExpression& expression, const std::string& column,
      std::type_index type) {
    auto term = translate(expression.get_expression());
    set_translation(Viper::access(term.m_expression, column), type);
  }

  inline void SqlTranslator::translate_symbol(
      const Beam::MemberAccessExpression& expression, const std::string& column,
      std::type_index type) {
    translate(expression.get_expression());
    set_translation(Viper::sym(column), type);
  }

  inline void SqlTranslator::translate_embedded(
      const Beam::MemberAccessExpression& expression, std::type_index type) {
    translate(expression.get_expression());
    set_translation(Viper::sym(""), type);
  }

  inline void SqlTranslator::translate_ticker_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "symbol" || expression.get_name() == "venue") {
      translate_access(expression, expression.get_name(), typeid(std::string));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_ticker_info_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "ticker") {
      translate_embedded(expression, typeid(Ticker));
    } else if(expression.get_name() == "name" ||
        expression.get_name() == "sector") {
      translate_access(expression, expression.get_name(), typeid(std::string));
    } else if(expression.get_name() == "board_lot") {
      translate_access(expression, expression.get_name(), typeid(Quantity));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "price") {
      translate_access(expression, expression.get_name(), typeid(Money));
    } else if(expression.get_name() == "size") {
      translate_access(expression, expression.get_name(), typeid(Quantity));
    } else if(expression.get_name() == "side") {
      translate_access(expression, expression.get_name(), typeid(Side));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_bbo_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "bid" || expression.get_name() == "ask") {
      translate_access(expression, expression.get_name(), typeid(Quote));
    } else if(expression.get_name() == "timestamp") {
      translate_access(
        expression, "timestamp", typeid(boost::posix_time::ptime));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_book_quote_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "mpid") {
      translate_access(expression, expression.get_name(), typeid(std::string));
    } else if(expression.get_name() == "timestamp") {
      translate_access(
        expression, expression.get_name(), typeid(boost::posix_time::ptime));
    } else if(expression.get_name() == "is_primary_mpid") {
      translate_access(expression, "is_primary", typeid(bool));
    } else if(expression.get_name() == "venue") {
      translate_access(expression, "quote_venue", typeid(Venue));
    } else if(expression.get_name() == "quote") {
      translate_embedded(expression, typeid(Quote));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_order_imbalance_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "ticker") {
      translate_embedded(expression, typeid(Ticker));
    } else if(expression.get_name() == "side") {
      translate_access(expression, expression.get_name(), typeid(Side));
    } else if(expression.get_name() == "size") {
      translate_access(expression, expression.get_name(), typeid(Quantity));
    } else if(expression.get_name() == "reference_price") {
      translate_access(expression, "price", typeid(Money));
    } else if(expression.get_name() == "timestamp") {
      translate_access(
        expression, "timestamp", typeid(boost::posix_time::ptime));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_ticker_status_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "state") {
      translate_access(
        expression, expression.get_name(), typeid(std::string));
    } else if(expression.get_name() == "flags") {
      translate_access(expression, expression.get_name(), typeid(int));
    } else if(expression.get_name() == "timestamp") {
      translate_access(
        expression, expression.get_name(), typeid(boost::posix_time::ptime));
    } else if(expression.get_name() == "venue") {
      translate_access(expression, "status_venue", typeid(Venue));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_time_and_sale_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "timestamp") {
      translate_access(
        expression, expression.get_name(), typeid(boost::posix_time::ptime));
    } else if(expression.get_name() == "price") {
      translate_access(expression, expression.get_name(), typeid(Money));
    } else if(expression.get_name() == "size") {
      translate_access(expression, expression.get_name(), typeid(Quantity));
    } else if(expression.get_name() == "market_center") {
      translate_access(expression, "market", typeid(std::string));
    } else if(expression.get_name() == "buyer_mpid" ||
        expression.get_name() == "seller_mpid") {
      translate_access(expression, expression.get_name(), typeid(std::string));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_order_fields_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "ticker") {
      translate_embedded(expression, typeid(Ticker));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::translate_order_info_member_access_expression(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "fields") {
      translate_embedded(expression, typeid(OrderFields));
    } else if(expression.get_name() == "order_id") {
      translate_access(expression, expression.get_name(), typeid(OrderId));
    } else if(expression.get_name() == "shorting_flag" ||
        expression.get_name() == "is_live") {
      translate_access(expression, expression.get_name(), typeid(bool));
    } else if(expression.get_name() == "timestamp") {
      translate_access(
        expression, expression.get_name(), typeid(boost::posix_time::ptime));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }

  inline void SqlTranslator::
      translate_account_modification_request_member_access_expression(
        const Beam::MemberAccessExpression& expression) {
    if(expression.get_name() == "id" || expression.get_name() == "type" ||
        expression.get_name() == "account" ||
        expression.get_name() == "submission_account" ||
        expression.get_name() == "status") {
      translate_symbol(expression, expression.get_name(), typeid(int));
    } else if(expression.get_name() == "timestamp" ||
        expression.get_name() == "effective_date" ||
        expression.get_name() == "last_update_timestamp") {
      translate_symbol(
        expression, expression.get_name(), typeid(boost::posix_time::ptime));
    } else {
      Beam::SqlTranslator<QueryTypes>::visit(expression);
    }
  }
}

#endif
