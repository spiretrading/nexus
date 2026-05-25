#ifndef NEXUS_QUERIES_QUOTE_ACCESSOR_HPP
#define NEXUS_QUERIES_QUOTE_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/Quote.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to a Quote.
   */
  class QuoteAccessor {
    public:

      /**
       * Constructs a QuoteAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed QuoteAccessor.
       */
      static QuoteAccessor from_parameter(int index);

      /**
       * Constructs a QuoteAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit QuoteAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the price member. */
      Beam::MemberAccessExpression get_price() const;

      /** Returns an accessor for the size member. */
      Beam::MemberAccessExpression get_size() const;

      /** Returns an accessor for the side member. */
      Beam::MemberAccessExpression get_side() const;

    private:
      Beam::Expression m_expression;
  };

  inline QuoteAccessor QuoteAccessor::from_parameter(int index) {
    return QuoteAccessor(Beam::ParameterExpression(index, typeid(Quote)));
  }

  inline QuoteAccessor::QuoteAccessor(Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression QuoteAccessor::get_price() const {
    return Beam::MemberAccessExpression("price", typeid(Money), m_expression);
  }

  inline Beam::MemberAccessExpression QuoteAccessor::get_size() const {
    return Beam::MemberAccessExpression("size", typeid(Quantity), m_expression);
  }

  inline Beam::MemberAccessExpression QuoteAccessor::get_side() const {
    return Beam::MemberAccessExpression("side", typeid(Side), m_expression);
  }
}

#endif
