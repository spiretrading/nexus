#ifndef SPIRE_TRANSACTION_LOG_MODEL_HPP
#define SPIRE_TRANSACTION_LOG_MODEL_HPP
#include "Spire/Spire/ModelTransactionLog.hpp"
#include "Spire/Spire/TransactionalModel.hpp"

namespace Spire {

  /** A model that publishes its operations through a transaction log. */
  template<std::derived_from<TransactionalModel> M>
  class TransactionLogModel : public M {
    public:
      boost::signals2::connection connect_operation_signal(
        const typename M::OperationSignal::slot_type& slot) const override;
      void start_transaction() override;
      void end_transaction() override;

    protected:
      ModelTransactionLog<M> m_transaction;

      TransactionLogModel() = default;
  };

  template<std::derived_from<TransactionalModel> M>
  boost::signals2::connection TransactionLogModel<M>::connect_operation_signal(
      const typename M::OperationSignal::slot_type& slot) const {
    return m_transaction.connect_operation_signal(slot);
  }

  template<std::derived_from<TransactionalModel> M>
  void TransactionLogModel<M>::start_transaction() {
    m_transaction.start();
  }

  template<std::derived_from<TransactionalModel> M>
  void TransactionLogModel<M>::end_transaction() {
    m_transaction.end();
  }
}

#endif
