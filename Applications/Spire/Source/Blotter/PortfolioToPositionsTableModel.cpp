#include "Spire/Blotter/PortfolioToPositionsTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

PortfolioToPositionsTableModel::PortfolioToPositionsTableModel(
  std::shared_ptr<PortfolioModel> portfolio) {}

int PortfolioToPositionsTableModel::get_row_size() const {
  return 0;
}

int PortfolioToPositionsTableModel::get_column_size() const {
  return 0;
}

AnyRef PortfolioToPositionsTableModel::at(int row, int column) const {
  return {};
}

connection PortfolioToPositionsTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return {};
}
