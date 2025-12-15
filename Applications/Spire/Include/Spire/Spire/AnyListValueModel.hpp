#ifndef SPIRE_ANY_LIST_VALUE_MODEL_HPP
#define SPIRE_ANY_LIST_VALUE_MODEL_HPP
#include <memory>
#include "Spire/Spire/ListIndexTracker.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /**
   * Implements a ValueModel providing a view over the value in an AnyListModel
   * at a specified index.
   */
  class AnyListValueModel : public ValueModel<std::any> {
    public:
      using Type = ValueModel<std::any>::Type;
      using UpdateSignal = ValueModel<std::any>::UpdateSignal;

      /**
       * Constructs an AnyListValueModel from a specified index into an
       * AnyListModel.
       * @param source The AnyListModel to view.
       * @param index The index of the value in the <i>source</i> to view. 
       */
      AnyListValueModel(std::shared_ptr<AnyListModel> source, int index);

      QValidator::State get_state() const override;
      const Type& get() const override;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<AnyListModel> m_source;
      LocalValueModel<std::any> m_current;
      ListIndexTracker m_index;
      boost::signals2::scoped_connection m_source_connection;

      void on_operation(const AnyListModel::Operation& operation);
  };
}

#endif
