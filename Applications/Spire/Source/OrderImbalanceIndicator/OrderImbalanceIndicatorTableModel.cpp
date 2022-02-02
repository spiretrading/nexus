#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorTabelModel.hpp"

using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderImbalanceIndicatorTableModel::OrderImbalanceIndicatorTableModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source(std::move(source)) {

}
  
void OrderImbalanceIndicatorTableModel::set_interval(
    const TimeInterval& interval) {

}

void OrderImbalanceIndicatorTableModel::set_offset(
    const time_duration& offset) {

}

int OrderImbalanceIndicatorTableModel::get_row_size() const {

}

int OrderImbalanceIndicatorTableModel::get_column_size() const {

}

const std::any&
    OrderImbalanceIndicatorTableModel::at(int row, int column) const {

}

connection OrderImbalanceIndicatorTableModel::connect_operation_signal(
    const typename OperationSignal::slot_type& slot) const {
  return m_operation_signal.connect(slot);
}
