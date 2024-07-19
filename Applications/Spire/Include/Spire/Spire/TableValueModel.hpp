#ifndef SPIRE_TABLE_VALUE_MODEL_HPP
#define SPIRE_TABLE_VALUE_MODEL_HPP
#include "Spire/Spire/ColumnViewListModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /**
   * Makes a model that provides a view into a table's value.
   * @param table The table to view.
   * @param row The index of the row to view.
   * @param column The index of the column to view.
   */
  template<typename T>
  std::shared_ptr<ValueModel<T>> make_table_value_model(
      std::shared_ptr<TableModel> table, int row, int column) {
    return make_list_value_model(
      std::make_shared<ColumnViewListModel<T>>(std::move(table), column), row);
  }
}

#endif
