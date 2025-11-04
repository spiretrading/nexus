#ifndef NEXUS_QUERIES_SECURITY_ACCESSOR_HPP
#define NEXUS_QUERIES_SECURITY_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/Definitions/Security.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to a Security.
   */
  class SecurityAccessor {
    public:

      /**
       * Constructs a SecurityAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed SecurityAccessor.
       */
      static SecurityAccessor from_parameter(int index);

      /**
       * Constructs a SecurityAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit SecurityAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the symbol member. */
      Beam::MemberAccessExpression get_symbol() const;

      /** Returns an accessor for the venue member. */
      Beam::MemberAccessExpression get_venue() const;

    private:
      Beam::Expression m_expression;
  };

  inline SecurityAccessor SecurityAccessor::from_parameter(int index) {
    return SecurityAccessor(
      Beam::ParameterExpression(index, typeid(Security)));
  }

  inline SecurityAccessor::SecurityAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression SecurityAccessor::get_symbol() const {
    return Beam::MemberAccessExpression(
      "symbol", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression SecurityAccessor::get_venue() const {
    return Beam::MemberAccessExpression(
      "venue", typeid(std::string), m_expression);
  }
}

#endif
