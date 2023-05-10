#include "Spire/Spire/ExportTable.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam::TimeService;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  auto to_string(ptime time) {
    auto local_time = ToLocalTime(time);
    auto ss = std::ostringstream();
    auto facet = new time_facet();
    facet->format("%Y-%m-%d %H:%M:%S%F");
    ss.imbue(std::locale(std::locale::classic(), facet));
    ss << local_time;
    return ss.str();
  }
}

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
      if(value_type == typeid(ptime)) {
        out << to_string(std::any_cast<ptime>(to_any(value)));
      } else if(value_type == typeid(bool) || value_type == typeid(int) ||
          value_type == typeid(double) || value_type == typeid(date) ||
          value_type == typeid(time_duration) || value_type == typeid(Money) ||
          value_type == typeid(Quantity)) {
        if(value_type == typeid(Quantity)) {
          value = static_cast<double>(std::any_cast<Quantity>(to_any(value)));
        }
        auto locale = QLocale();
        locale.setNumberOptions(QLocale::OmitGroupSeparator);
        out << CustomVariantItemDelegate().displayText(
          to_qvariant(to_any(value)), locale).toStdString();
      } else {
        out << "\"" << displayText(value).toStdString() << "\"";
      }
    }
  }
}
