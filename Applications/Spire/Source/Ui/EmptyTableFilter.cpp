#include "Spire/Ui/EmptyTableFilter.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TableFilter::Filter EmptyTableFilter::get_filter(int column) {
  return TableFilter::Filter::NONE;
}

QWidget* EmptyTableFilter::make_filter_widget(int column, QWidget& parent) {
  return nullptr;
}

bool EmptyTableFilter::is_filtered(const TableModel& model, int row) const {
  return false;
}

connection EmptyTableFilter::connect_filter_signal(
    const FilterSignal::slot_type& slot) const {
  return {};
}
