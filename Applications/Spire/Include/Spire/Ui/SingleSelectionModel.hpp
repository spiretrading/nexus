#ifndef SPIRE_SINGLE_SELECTION_MODEL_HPP
#define SPIRE_SINGLE_SELECTION_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/ListModelTransactionLog.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a ListModel used when only a single item can be selected. */
  class SingleSelectionModel : public ListModel<int> {
    public:

      /** Constructs an empty SingleSelectionModel. */
      SingleSelectionModel() = default;

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      QValidator::State insert(const Type& value, int index) override;

      QValidator::State move(int source, int destination) override;

      QValidator::State remove(int index) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      boost::optional<int> m_value;
      ListModelTransactionLog<int> m_transaction;
  };
}

#endif
