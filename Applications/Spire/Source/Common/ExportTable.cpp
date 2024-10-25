#include "Spire/Spire/ExportTable.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam::TimeService;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

void Spire::export_table_as_csv(const TableModel& table,
    const std::vector<QString>& headers, std::ostream& out) {
  auto locale = QLocale();
  locale.setNumberOptions(QLocale::OmitGroupSeparator);
  for(auto i = headers.begin(); i != headers.end(); ++i) {
    out << '\"' << i->toStdString() << '\"';
    if(i == headers.end() - 1) {
      out << '\n';
    } else {
      out << ',';
    }
  }
  for(auto i = 0; i < table.get_row_size(); ++i) {
    if(i != 0) {
      out << '\n';
    }
    for(auto j = 0; j < table.get_column_size(); ++j) {
      if(j != 0) {
        out << ',';
      }
      auto value = table.at(i, j);
      auto& value_type = value.get_type();
      if(value_type == typeid(ptime)) {
        auto local_time = ToLocalTime(std::any_cast<ptime>(to_any(value)));
        out << to_iso_extended_string(local_time).replace(10, 1, " ");
      } else if(value_type == typeid(bool) || value_type == typeid(int) ||
          value_type == typeid(double) || value_type == typeid(date) ||
          value_type == typeid(time_duration) || value_type == typeid(Money) ||
          value_type == typeid(Quantity)) {
        out << to_text(value, locale).toStdString();
      } else {
        out << '\"' << to_text(value, locale).toStdString() << '\"';
      }
    }
  }
}
