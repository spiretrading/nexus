#include "Spire/Ui/TableFilter.hpp"

using namespace boost::signals2;
using namespace Spire;

connection TableFilter::connect_rows_signal(
    const RowsSignal::slot_type& slot) const {
  return {};
}
