#ifndef NEXUS_ACCOUNT_MODIFICATION_REQUEST_SQL_TRANSLATOR_HPP
#define NEXUS_ACCOUNT_MODIFICATION_REQUEST_SQL_TRANSLATOR_HPP
#include <string>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/SqlTranslator.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

namespace Nexus {

  /** Translates an AccountModificationRequest Expression into SQL. */
  class AccountModificationRequestSqlTranslator : public Beam::SqlTranslator {
    public:
      using Beam::SqlTranslator::SqlTranslator;

    protected:
      void visit(const Beam::MemberAccessExpression& expression) override;
  };

  /**
   * Translates an AccountModificationRequest Expression into an SQL query.
   * @param parameter The parameter/table name.
   * @param expression The Expression to translate.
   * @return The SQL query representing the <i>expression</i>.
   */
  inline auto make_account_modification_request_sql_query(
      std::string parameter, Beam::Expression expression) {
    return Beam::make_sql_query<AccountModificationRequestSqlTranslator>(
      std::move(parameter), std::move(expression));
  }

  inline void AccountModificationRequestSqlTranslator::visit(
      const Beam::MemberAccessExpression& expression) {
    if(expression.get_expression().get_type() !=
        typeid(AccountModificationRequest)) {
      Beam::SqlTranslator::visit(expression);
      return;
    }
    auto& name = expression.get_name();
    if(name == "id" || name == "type" || name == "account" ||
        name == "submission_account" || name == "timestamp" ||
        name == "effective_date") {
      get_translation() = Viper::sym(name);
    } else {
      Beam::SqlTranslator::visit(expression);
    }
  }
}

#endif
