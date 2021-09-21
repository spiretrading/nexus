#ifndef SPIRE_LIST_VALUE_MODEL_HPP
#define SPIRE_LIST_VALUE_MODEL_HPP
#include <any>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a ValueModel providing a view over the value in a ListModel at
   * a specified index.
   */
  class ListValueModel : public ValueModel<std::any> {
    public:

      /** 
       * Constructs a ListValueModel from a specified index into a ListModel.
       * @param source The ListModel to view.
       * @param index The index of the value in the <i>source</i> to view. 
       */
      ListValueModel(std::shared_ptr<ListModel> source, int index);

      QValidator::State get_state() const override;

      const Type& get_current() const override;

      QValidator::State set_current(const Type& value) override;

      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const override;

    private:
      mutable CurrentSignal m_current_signal;
      std::shared_ptr<ListModel> m_source;
      int m_index;
      LocalValueModel<std::any> m_value;
      boost::signals2::scoped_connection m_operation_connection;

      void on_operation(const ListModel::Operation& operation);
  };
}

#endif
