#ifndef NEXUS_QUERIES_ORDER_INFO_ACCESSOR_HPP
#define NEXUS_QUERIES_ORDER_INFO_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/OrderExecutionService/OrderInfo.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to OrderInfo.
   */
  class OrderInfoAccessor {
    public:

      /**
       * Constructs an OrderInfoAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed OrderInfoAccessor.
       */
      static OrderInfoAccessor from_parameter(int index);

      /**
       * Constructs an OrderInfoAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit OrderInfoAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the fields member. */
      Beam::MemberAccessExpression get_fields() const;

      /** Returns an accessor for the submission account member. */
      Beam::MemberAccessExpression get_submission_account() const;

      /** Returns an accessor for the order id member. */
      Beam::MemberAccessExpression get_order_id() const;

      /** Returns an accessor for the shorting flag member. */
      Beam::MemberAccessExpression get_shorting_flag() const;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

      /** Returns an accessor for the is_live member. */
      Beam::MemberAccessExpression is_live() const;

    private:
      Beam::Expression m_expression;
  };

  inline OrderInfoAccessor OrderInfoAccessor::from_parameter(int index) {
    return OrderInfoAccessor(
      Beam::ParameterExpression(index, typeid(OrderInfo)));
  }

  inline OrderInfoAccessor::OrderInfoAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression OrderInfoAccessor::get_fields() const {
    return Beam::MemberAccessExpression(
      "fields", typeid(OrderFields), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderInfoAccessor::get_submission_account() const {
    return Beam::MemberAccessExpression(
      "submission_account", typeid(Beam::DirectoryEntry), m_expression);
  }

  inline Beam::MemberAccessExpression OrderInfoAccessor::get_order_id() const {
    return Beam::MemberAccessExpression(
      "order_id", typeid(OrderId), m_expression);
  }

  inline Beam::MemberAccessExpression
      OrderInfoAccessor::get_shorting_flag() const {
    return Beam::MemberAccessExpression(
      "shorting_flag", typeid(bool), m_expression);
  }

  inline Beam::MemberAccessExpression OrderInfoAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }

  inline Beam::MemberAccessExpression OrderInfoAccessor::is_live() const {
    return Beam::MemberAccessExpression("is_live", typeid(bool), m_expression);
  }
}

#endif
