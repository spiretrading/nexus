#ifndef NEXUS_QUERIES_TICKER_ACCESSOR_HPP
#define NEXUS_QUERIES_TICKER_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/Ticker.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to a Ticker.
   */
  class TickerAccessor {
    public:

      /**
       * Constructs a TickerAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed TickerAccessor.
       */
      static TickerAccessor from_parameter(int index);

      /**
       * Constructs a TickerAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit TickerAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the symbol member. */
      Beam::MemberAccessExpression get_symbol() const;

      /** Returns an accessor for the venue member. */
      Beam::MemberAccessExpression get_venue() const;

    private:
      Beam::Expression m_expression;
  };

  inline TickerAccessor TickerAccessor::from_parameter(int index) {
    return TickerAccessor(Beam::ParameterExpression(index, typeid(Ticker)));
  }

  inline TickerAccessor::TickerAccessor(Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression TickerAccessor::get_symbol() const {
    return Beam::MemberAccessExpression(
      "symbol", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression TickerAccessor::get_venue() const {
    return Beam::MemberAccessExpression(
      "venue", typeid(std::string), m_expression);
  }
}

#endif
