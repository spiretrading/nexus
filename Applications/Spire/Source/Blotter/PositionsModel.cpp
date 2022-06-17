#include "Spire/Blotter/PositionsModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::RiskService;
using namespace Spire;


PositionsModel::PositionsModel(
  const InventorySnapshot& snapshot, std::shared_ptr<OrderListModel> orders,
  std::shared_ptr<ValuationModel> valuation) {}

std::vector<PositionsModel::Entry> PositionsModel::get_positions() const {
  return {};
}

connection PositionsModel::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return {};
}
