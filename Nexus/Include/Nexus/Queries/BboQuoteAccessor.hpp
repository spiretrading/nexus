#ifndef NEXUS_QUERIES_BBO_QUOTE_ACCESSOR_HPP
#define NEXUS_QUERIES_BBO_QUOTE_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/BboQuote.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to a BboQuote.
   */
  class BboQuoteAccessor {
    public:

      /**
       * Constructs a BboQuoteAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed BboQuoteAccessor.
       */
      static BboQuoteAccessor from_parameter(int index);

      /**
       * Constructs a BboQuoteAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit BboQuoteAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the bid member. */
      Beam::MemberAccessExpression get_bid() const;

      /** Returns an accessor for the ask member. */
      Beam::MemberAccessExpression get_ask() const;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

    private:
      Beam::Expression m_expression;
  };

  inline BboQuoteAccessor BboQuoteAccessor::from_parameter(int index) {
    return BboQuoteAccessor(Beam::ParameterExpression(index, typeid(BboQuote)));
  }

  inline BboQuoteAccessor::BboQuoteAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression BboQuoteAccessor::get_bid() const {
    return Beam::MemberAccessExpression("bid", typeid(Quote), m_expression);
  }

  inline Beam::MemberAccessExpression BboQuoteAccessor::get_ask() const {
    return Beam::MemberAccessExpression("ask", typeid(Quote), m_expression);
  }

  inline Beam::MemberAccessExpression
      BboQuoteAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }
}

#endif
