#ifndef NEXUS_ACCOUNT_MODIFICATION_REQUEST_ACCESSOR_HPP
#define NEXUS_ACCOUNT_MODIFICATION_REQUEST_ACCESSOR_HPP
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/ParameterExpression.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

namespace Nexus {

  /**
   * Provides member accessors for an Expression evaluating to an
   * AccountModificationRequest.
   */
  class AccountModificationRequestAccessor {
    public:

      /**
       * Constructs an AccountModificationRequestAccessor from a parameter.
       * @param index The index of the parameter.
       * @return The constructed AccountModificationRequestAccessor.
       */
      static AccountModificationRequestAccessor from_parameter(int index);

      /**
       * Constructs an AccountModificationRequestAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit AccountModificationRequestAccessor(
        Beam::Expression expression) noexcept;

      /** Returns an accessor for the id member. */
      Beam::MemberAccessExpression get_id() const;

      /** Returns an accessor for the type member. */
      Beam::MemberAccessExpression get_type() const;

      /** Returns an accessor for the account member. */
      Beam::MemberAccessExpression get_account() const;

      /** Returns an accessor for the submission account member. */
      Beam::MemberAccessExpression get_submission_account() const;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

      /** Returns an accessor for the effective date member. */
      Beam::MemberAccessExpression get_effective_date() const;

    private:
      Beam::Expression m_expression;
  };

  inline AccountModificationRequestAccessor
      AccountModificationRequestAccessor::from_parameter(int index) {
    return AccountModificationRequestAccessor(Beam::ParameterExpression(
      index, typeid(AccountModificationRequest)));
  }

  inline AccountModificationRequestAccessor::
    AccountModificationRequestAccessor(Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression
      AccountModificationRequestAccessor::get_id() const {
    return Beam::MemberAccessExpression("id", typeid(int), m_expression);
  }

  inline Beam::MemberAccessExpression
      AccountModificationRequestAccessor::get_type() const {
    return Beam::MemberAccessExpression("type", typeid(int), m_expression);
  }

  inline Beam::MemberAccessExpression
      AccountModificationRequestAccessor::get_account() const {
    return Beam::MemberAccessExpression("account", typeid(int), m_expression);
  }

  inline Beam::MemberAccessExpression
      AccountModificationRequestAccessor::get_submission_account() const {
    return Beam::MemberAccessExpression(
      "submission_account", typeid(int), m_expression);
  }

  inline Beam::MemberAccessExpression
      AccountModificationRequestAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }

  inline Beam::MemberAccessExpression
      AccountModificationRequestAccessor::get_effective_date() const {
    return Beam::MemberAccessExpression(
      "effective_date", typeid(boost::posix_time::ptime), m_expression);
  }
}

#endif
