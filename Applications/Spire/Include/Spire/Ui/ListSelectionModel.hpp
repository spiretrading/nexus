#ifndef SPIRE_LIST_SELECTION_MODEL_HPP
#define SPIRE_LIST_SELECTION_MODEL_HPP
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /**
   * Implements a ListView's selection model that allows for switching between
   * between different selection modes.
   */
  class ListSelectionModel : public ListModel<int> {
    public:

      /** The different selection modes supported by this model. */
      enum class Mode {

        /** No items are selected, the model is always empty. */
        NONE,

        /** At most a single item is selected at a time. */
        SINGLE,

        /** Multiple items may be selected. */
        MULTI
      };

      /**
       * Constructs an empty ListSelectionModel in the SINGLE selection mode.
       */
      ListSelectionModel();

      /** Returns the selection mode. */
      Mode get_mode() const;

      /** Sets the selection mode. */
      void set_mode(Mode mode);

      int get_size() const override;
      const int& get(int index) const override;
      QValidator::State set(int index, const int& value) override;
      QValidator::State insert(const int& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      mutable OperationSignal m_operation_signal;
      Mode m_mode;
      std::shared_ptr<ListModel<int>> m_model;
  };
}

#endif
