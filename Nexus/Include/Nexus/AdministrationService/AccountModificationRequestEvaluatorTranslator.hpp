#ifndef NEXUS_ACCOUNT_MODIFICATION_REQUEST_EVALUATOR_TRANSLATOR_HPP
#define NEXUS_ACCOUNT_MODIFICATION_REQUEST_EVALUATOR_TRANSLATOR_HPP
#include <memory>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <Beam/Queries/FunctionEvaluatorNode.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Utilities/Casts.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountModificationRequestQueryTypes.hpp"

namespace Nexus {

  /** Translates an AccountModificationRequest Expression into an Evaluator. */
  class AccountModificationRequestEvaluatorTranslator :
      public Beam::EvaluatorTranslator<AccountModificationRequestQueryTypes> {
    public:
      std::unique_ptr<
        Beam::EvaluatorTranslator<AccountModificationRequestQueryTypes>>
          make_translator() const override;

    protected:
      void visit(const Beam::MemberAccessExpression& expression) override;
  };

  inline std::unique_ptr<
      Beam::EvaluatorTranslator<AccountModificationRequestQueryTypes>>
        AccountModificationRequestEvaluatorTranslator::
          make_translator() const {
    return std::make_unique<AccountModificationRequestEvaluatorTranslator>();
  }

  inline void AccountModificationRequestEvaluatorTranslator::visit(
      const Beam::MemberAccessExpression& expression) {
    using Base =
      Beam::EvaluatorTranslator<AccountModificationRequestQueryTypes>;
    if(expression.get_expression().get_type() !=
        typeid(AccountModificationRequest)) {
      Base::visit(expression);
      return;
    }
    expression.get_expression().apply(*this);
    auto request = Beam::static_pointer_cast<
      Beam::EvaluatorNode<AccountModificationRequest>>(get_evaluator());
    auto& name = expression.get_name();
    if(name == "id") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return request.get_id();
        }, std::move(request)));
    } else if(name == "type") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return static_cast<int>(request.get_type());
        }, std::move(request)));
    } else if(name == "account") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return static_cast<int>(request.get_account().m_id);
        }, std::move(request)));
    } else if(name == "submission_account") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return static_cast<int>(request.get_submission_account().m_id);
        }, std::move(request)));
    } else if(name == "timestamp") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return request.get_timestamp();
        }, std::move(request)));
    } else if(name == "effective_date") {
      set_evaluator(Beam::make_function_evaluator_node(
        [] (const AccountModificationRequest& request) {
          return request.get_effective_date();
        }, std::move(request)));
    } else {
      Base::visit(expression);
    }
  }
}

#endif
