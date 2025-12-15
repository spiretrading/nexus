#ifndef SPIRE_TABLE_SELECTION_MODEL_HPP
#define SPIRE_TABLE_SELECTION_MODEL_HPP
#include <memory>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/TableIndex.hpp"

namespace Spire {

  /** Models a table's selected items, rows and columns. */
  class TableSelectionModel {
    public:
      using Index = TableIndex;

      /** The type of model used to select items. */
      using ItemSelectionModel = ListModel<Index>;

      /** The type of model used to select rows. */
      using RowSelectionModel = ListModel<int>;

      /** The type of model used to select columns. */
      using ColumnSelectionModel = ListModel<int>;

      /**
       * Constructs a TableSelectionModel.
       * @param item_selection The list of selected items.
       * @param row_selection The list of selected rows.
       * @param column_selection The list of selected columns.
       */
      TableSelectionModel(std::shared_ptr<ListModel<Index>> item_selection,
        std::shared_ptr<ListModel<int>> row_selection,
        std::shared_ptr<ListModel<int>> column_selection);

      /** Returns the list of selected items. */
      std::shared_ptr<const ItemSelectionModel> get_item_selection() const;

      /** Returns the list of selected items. */
      const std::shared_ptr<ItemSelectionModel>& get_item_selection();

      /** Returns the list of selected rows. */
      std::shared_ptr<const RowSelectionModel> get_row_selection() const;

      /** Returns the list of selected rows. */
      const std::shared_ptr<RowSelectionModel>& get_row_selection();

      /** Returns the list of selected columns. */
      std::shared_ptr<const ColumnSelectionModel> get_column_selection() const;

      /** Returns the list of selected columns. */
      const std::shared_ptr<ColumnSelectionModel>& get_column_selection();

      /**
       * Performs a transaction on all of the item, row and column selections.
       */
      template<typename F>
      decltype(auto) transact(F&& f);

    private:
      std::shared_ptr<ItemSelectionModel> m_item_selection;
      std::shared_ptr<RowSelectionModel> m_row_selection;
      std::shared_ptr<ColumnSelectionModel> m_column_selection;
  };

  template<typename F>
  decltype(auto) TableSelectionModel::transact(F&& f) {
    return m_item_selection->transact([&] {
      return m_row_selection->transact([&] {
        return m_column_selection->transact(std::forward<F>(f));
      });
    });
  }
}

#endif
