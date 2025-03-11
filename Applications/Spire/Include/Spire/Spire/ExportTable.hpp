#ifndef SPIRE_EXPORT_TABLE_HPP
#define SPIRE_EXPORT_TABLE_HPP
#include <ostream>
#include "Spire/Spire/TableModel.hpp"

namespace Spire {

  /**
   * Exports a table model in CSV format.
   * @param table The table model to export.
   * @param headers The table headers to export.
   * @param out The stream to output in CSV format.
   */
  void export_table_as_csv(const TableModel& table,
    const std::vector<QString>& headers, std::ostream& out);

}

#endif
