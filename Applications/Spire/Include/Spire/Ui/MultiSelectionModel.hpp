#ifndef SPIRE_MULTI_SELECTION_MODEL_HPP
#define SPIRE_MULTI_SELECTION_MODEL_HPP
#include <unordered_set>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ListModel used when multiple items can be selected. Ensures
   * that no duplicate values are contained.
   */
  class MultiSelectionModel : public ListModel<int> {
    public:

      /** Constructs an empty MultiSelectionModel. */
      MultiSelectionModel() = default;

      int get_size() const override;

      const Type& get(int index) const override;

      QValidator::State set(int index, const Type& value) override;

      QValidator::State insert(const Type& value, int index) override;

      QValidator::State move(int source, int destination) override;

      QValidator::State remove(int index) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    private:
      std::unordered_set<int> m_selection;
      ArrayListModel<int> m_list;
  };
}

#endif
