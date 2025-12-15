#ifndef NEXUS_QUERIES_TIME_AND_SALE_ACCESSOR_HPP
#define NEXUS_QUERIES_TIME_AND_SALE_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/TimeAndSale.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to a TimeAndSale.
   */
  class TimeAndSaleAccessor {
    public:

      /**
       * Constructs a TimeAndSaleAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed TimeAndSaleAccessor.
       */
      static TimeAndSaleAccessor from_parameter(int index);

      /**
       * Constructs a TimeAndSaleAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit TimeAndSaleAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

      /** Returns an accessor for the price member. */
      Beam::MemberAccessExpression get_price() const;

      /** Returns an accessor for the size member. */
      Beam::MemberAccessExpression get_size() const;

      /** Returns an accessor for the market center member. */
      Beam::MemberAccessExpression get_market_center() const;

      /** Returns an accessor for the buyer MPID member. */
      Beam::MemberAccessExpression get_buyer_mpid() const;

      /** Returns an accessor for the seller MPID member. */
      Beam::MemberAccessExpression get_seller_mpid() const;

    private:
      Beam::Expression m_expression;
  };

  inline TimeAndSaleAccessor TimeAndSaleAccessor::from_parameter(int index) {
    return TimeAndSaleAccessor(
      Beam::ParameterExpression(index, typeid(TimeAndSale)));
  }

  inline TimeAndSaleAccessor::TimeAndSaleAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }

  inline Beam::MemberAccessExpression TimeAndSaleAccessor::get_price() const {
    return Beam::MemberAccessExpression("price", typeid(Money), m_expression);
  }

  inline Beam::MemberAccessExpression TimeAndSaleAccessor::get_size() const {
    return Beam::MemberAccessExpression("size", typeid(Quantity), m_expression);
  }

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_market_center() const {
    return Beam::MemberAccessExpression(
      "market_center", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_buyer_mpid() const {
    return Beam::MemberAccessExpression(
      "buyer_mpid", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_seller_mpid() const {
    return Beam::MemberAccessExpression(
      "seller_mpid", typeid(std::string), m_expression);
  }
}

#endif
