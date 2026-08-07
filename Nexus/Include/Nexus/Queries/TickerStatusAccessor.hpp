#ifndef NEXUS_QUERIES_TICKER_STATUS_ACCESSOR_HPP
#define NEXUS_QUERIES_TICKER_STATUS_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/TickerStatus.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to a TickerStatus.
   */
  class TickerStatusAccessor {
    public:

      /**
       * Constructs a TickerStatusAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed TickerStatusAccessor.
       */
      static TickerStatusAccessor from_parameter(int index);

      /**
       * Constructs a TickerStatusAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit TickerStatusAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the venue member. */
      Beam::MemberAccessExpression get_venue() const;

      /** Returns an accessor for the state member. */
      Beam::MemberAccessExpression get_state() const;

      /** Returns an accessor for the flags member as an int. */
      Beam::MemberAccessExpression get_flags() const;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

    private:
      Beam::Expression m_expression;
  };

  inline TickerStatusAccessor TickerStatusAccessor::from_parameter(int index) {
    return TickerStatusAccessor(
      Beam::ParameterExpression(index, typeid(TickerStatus)));
  }

  inline TickerStatusAccessor::TickerStatusAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression TickerStatusAccessor::get_venue() const {
    return Beam::MemberAccessExpression("venue", typeid(Venue), m_expression);
  }

  inline Beam::MemberAccessExpression TickerStatusAccessor::get_state() const {
    return Beam::MemberAccessExpression(
      "state", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression TickerStatusAccessor::get_flags() const {
    return Beam::MemberAccessExpression("flags", typeid(int), m_expression);
  }

  inline Beam::MemberAccessExpression
      TickerStatusAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }
}

#endif
