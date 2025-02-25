#ifndef SPIRE_LIST_INDEX_MODEL_HPP
#define SPIRE_LIST_INDEX_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Spire/ListIndexTracker.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Implements a ValueModel that keeps track of an index into a ListModel. */
  class ListIndexModel : public ValueModel<boost::optional<int>> {
    public:

      /**
       * Constructs a ListIndexModel with no initial index.
       * @param list The ListModel to index.
       */
      explicit ListIndexModel(std::shared_ptr<AnyListModel> list);

      /**
       * Constructs a ListIndexModel that indexes a specified ListModel.
       * @param list The ListModel to index.
       * @param index The current index. 
       */
      ListIndexModel(
        std::shared_ptr<AnyListModel> list, boost::optional<int> index);

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<AnyListModel> m_list;
      LocalValueModel<boost::optional<int>> m_index;
      ListIndexTracker m_tracker;
      boost::signals2::scoped_connection m_list_connection;

      void on_operation(const AnyListModel::Operation& operation);
  };
}

#endif
