#include "Spire/Spire/ExportTable.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

void Spire::export_table_as_csv(const TableModel& table, std::ostream& out) {
  for(auto i = 0; i < table.get_row_size(); ++i) {
    if(i != 0) {
      out << std::endl;
    }
    for(auto j = 0; j < table.get_column_size(); ++j) {
      if(j != 0) {
        out << ",";
      }
      auto value = table.at(i, j);
      auto& value_type = value.get_type();
      if(value_type == typeid(bool) || value_type == typeid(int) ||
          value_type == typeid(double) || value_type == typeid(date) ||
          value_type == typeid(ptime) || value_type == typeid(time_duration) ||
          value_type == typeid(Quantity) || value_type == typeid(Money)) {
        out << displayText(value).toStdString();
      } else {
        out << "\"" << displayText(value).toStdString() << "\"";
      }
    }
  }
}
