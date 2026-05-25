#ifndef NEXUS_QUERIES_BOOK_QUOTE_ACCESSOR_HPP
#define NEXUS_QUERIES_BOOK_QUOTE_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/BookQuote.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to a BookQuote.
   */
  class BookQuoteAccessor {
    public:

      /**
       * Constructs a BookQuoteAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed BookQuoteAccessor.
       */
      static BookQuoteAccessor from_parameter(int index);

      /**
       * Constructs a BookQuoteAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit BookQuoteAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the mpid member. */
      Beam::MemberAccessExpression get_mpid() const;

      /** Returns an accessor for the is_primary_mpid member. */
      Beam::MemberAccessExpression is_primary_mpid() const;

      /** Returns an accessor for the venue member. */
      Beam::MemberAccessExpression get_venue() const;

      /** Returns an accessor for the quote member. */
      Beam::MemberAccessExpression get_quote() const;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

    private:
      Beam::Expression m_expression;
  };

  inline BookQuoteAccessor BookQuoteAccessor::from_parameter(int index) {
    return BookQuoteAccessor(
      Beam::ParameterExpression(index, typeid(BookQuote)));
  }

  inline BookQuoteAccessor::BookQuoteAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression BookQuoteAccessor::get_mpid() const {
    return Beam::MemberAccessExpression(
      "mpid", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression
      BookQuoteAccessor::is_primary_mpid() const {
    return Beam::MemberAccessExpression(
      "is_primary_mpid", typeid(bool), m_expression);
  }

  inline Beam::MemberAccessExpression BookQuoteAccessor::get_venue() const {
    return Beam::MemberAccessExpression("venue", typeid(Venue), m_expression);
  }

  inline Beam::MemberAccessExpression BookQuoteAccessor::get_quote() const {
    return Beam::MemberAccessExpression("quote", typeid(Quote), m_expression);
  }

  inline Beam::MemberAccessExpression BookQuoteAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }
}

#endif
