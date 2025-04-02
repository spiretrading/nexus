#ifndef SPIRE_TABLE_CURRENT_INDEX_MODEL_HPP
#define SPIRE_TABLE_CURRENT_INDEX_MODEL_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableIndex.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Implements a ValueModel that keeps track of TableModel's current index. */
  class TableCurrentIndexModel :
      public ValueModel<boost::optional<TableIndex>> {
    public:

      /**
       * Constructs a TableCurrentIndexModel with no initial index.
       * @param table The TableModel to index.
       */
      explicit TableCurrentIndexModel(std::shared_ptr<TableModel> table);

      /**
       * Constructs a TableCurrentIndexModel that indexes a specified
       * TableModel.
       * @param table The TableModel to index.
       * @param index The current index.
       */
      TableCurrentIndexModel(
        std::shared_ptr<TableModel> table, boost::optional<TableIndex> index);

      const Type& get() const override;

      QValidator::State test(const Type& value) const override;

      QValidator::State set(const Type& value) override;

      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_table;
      LocalValueModel<boost::optional<TableIndex>> m_index;
      TableRowIndexTracker m_tracker;
      boost::signals2::scoped_connection m_connection;

      void on_operation(const TableModel::Operation& operation);
  };
}

#endif
