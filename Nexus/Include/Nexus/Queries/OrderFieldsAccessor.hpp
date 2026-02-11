#ifndef NEXUS_QUERIES_ORDER_FIELDS_ACCESSOR_HPP
#define NEXUS_QUERIES_ORDER_FIELDS_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to OrderFields.
   */
  class OrderFieldsAccessor {
    public:

      /**
       * Constructs an OrderFieldsAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed OrderFieldsAccessor.
       */
      static OrderFieldsAccessor from_parameter(int index);

      /**
       * Constructs an OrderFieldsAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit OrderFieldsAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the account member. */
      Beam::MemberAccessExpression get_account() const;

      /** Returns an accessor for the ticker member. */
      Beam::MemberAccessExpression get_ticker() const;

      /** Returns an accessor for the currency member. */
      Beam::MemberAccessExpression get_currency() const;

      /** Returns an accessor for the type member. */
      Beam::MemberAccessExpression get_type() const;

      /** Returns an accessor for the side member. */
      Beam::MemberAccessExpression get_side() const;

      /** Returns an accessor for the destination member. */
      Beam::MemberAccessExpression get_destination() const;

      /** Returns an accessor for the quantity member. */
      Beam::MemberAccessExpression get_quantity() const;

      /** Returns an accessor for the price member. */
      Beam::MemberAccessExpression get_price() const;

      /** Returns an accessor for the time in force member. */
      Beam::MemberAccessExpression get_time_in_force() const;

    private:
      Beam::Expression m_expression;
  };

  inline OrderFieldsAccessor OrderFieldsAccessor::from_parameter(int index) {
    return OrderFieldsAccessor(
      Beam::ParameterExpression(index, typeid(OrderFields)));
  }

  inline OrderFieldsAccessor::OrderFieldsAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression OrderFieldsAccessor::get_account() const {
    return Beam::MemberAccessExpression(
      "account", typeid(Beam::DirectoryEntry), m_expression);
  }

  inline Beam::MemberAccessExpression OrderFieldsAccessor::get_ticker() const {
    return Beam::MemberAccessExpression("ticker", typeid(Ticker), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderFieldsAccessor::get_currency() const {
    return Beam::MemberAccessExpression(
      "currency", typeid(Asset), m_expression);
  }

  inline Beam::MemberAccessExpression OrderFieldsAccessor::get_type() const {
    return Beam::MemberAccessExpression(
      "type", typeid(OrderType), m_expression);
  }

  inline Beam::MemberAccessExpression OrderFieldsAccessor::get_side() const {
    return Beam::MemberAccessExpression("side", typeid(Side), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderFieldsAccessor::get_destination() const {
    return Beam::MemberAccessExpression(
      "destination", typeid(Destination), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderFieldsAccessor::get_quantity() const {
    return Beam::MemberAccessExpression(
      "quantity", typeid(Quantity), m_expression);
  }

  inline Beam::MemberAccessExpression OrderFieldsAccessor::get_price() const {
    return Beam::MemberAccessExpression("price", typeid(Money), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderFieldsAccessor::get_time_in_force() const {
    return Beam::MemberAccessExpression(
      "time_in_force", typeid(TimeInForce), m_expression);
  }
}

#endif
