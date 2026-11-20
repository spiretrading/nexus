#ifndef SPIRE_TRANSACTIONAL_MODEL_HPP
#define SPIRE_TRANSACTIONAL_MODEL_HPP
#include <concepts>
#include <utility>
#include <boost/scope/scope_exit.hpp>

namespace Spire {

  /** A model whose operations can be grouped into transactions. */
  class TransactionalModel {
    public:
      virtual ~TransactionalModel() = default;

      /** Starts a transaction, joining any transaction already in progress. */
      virtual void start_transaction() = 0;

      /** Ends the most recently started transaction. */
      virtual void end_transaction() = 0;

      /**
       * Performs a transaction and returns the result of the callable.
       * @param transaction The operations to perform.
       */
      template<std::invocable F>
      decltype(auto) transact(F&& transaction);
  };

  template<std::invocable F>
  decltype(auto) TransactionalModel::transact(F&& transaction) {
    start_transaction();
    auto end = boost::scope::scope_exit([&] {
      end_transaction();
    });
    try {
      return std::forward<F>(transaction)();
    } catch(...) {
      end.set_active(false);
      end_transaction();
      throw;
    }
  }
}

#endif
