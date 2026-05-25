#ifndef NEXUS_QUERIES_ORDER_IMBALANCE_ACCESSOR_HPP
#define NEXUS_QUERIES_ORDER_IMBALANCE_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/OrderImbalance.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to an
   * OrderImbalance.
   */
  class OrderImbalanceAccessor {
    public:

      /**
       * Constructs an OrderImbalanceAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed OrderImbalanceAccessor.
       */
      static OrderImbalanceAccessor from_parameter(int index);

      /**
       * Constructs an OrderImbalanceAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit OrderImbalanceAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the ticker member. */
      Beam::MemberAccessExpression get_ticker() const;

      /** Returns an accessor for the side member. */
      Beam::MemberAccessExpression get_side() const;

      /** Returns an accessor for the size member. */
      Beam::MemberAccessExpression get_size() const;

      /** Returns an accessor for the reference_price member. */
      Beam::MemberAccessExpression get_reference_price() const;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

    private:
      Beam::Expression m_expression;
  };

  inline OrderImbalanceAccessor OrderImbalanceAccessor::from_parameter(
      int index) {
    return OrderImbalanceAccessor(
      Beam::ParameterExpression(index, typeid(OrderImbalance)));
  }

  inline OrderImbalanceAccessor::OrderImbalanceAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression
      OrderImbalanceAccessor::get_ticker() const {
    return Beam::MemberAccessExpression(
      "ticker", typeid(Ticker), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderImbalanceAccessor::get_side() const {
    return Beam::MemberAccessExpression("side", typeid(Side), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderImbalanceAccessor::get_size() const {
    return Beam::MemberAccessExpression(
      "size", typeid(Quantity), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderImbalanceAccessor::get_reference_price() const {
    return Beam::MemberAccessExpression(
      "reference_price", typeid(Money), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderImbalanceAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }
}

#endif
