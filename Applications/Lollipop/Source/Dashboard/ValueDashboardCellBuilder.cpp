#include "Spire/Dashboard/ValueDashboardCellBuilder.hpp"
#include "Spire/Dashboard/ValueDashboardCell.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

ValueDashboardCellBuilder::ValueDashboardCellBuilder(
    const DashboardCell::Value& value)
    : m_value{value} {}

std::unique_ptr<DashboardCell> ValueDashboardCellBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto cell = std::make_unique<ValueDashboardCell>();
  cell->SetBufferSize(1);
  cell->SetValue(m_value);
  return std::move(cell);
}

std::unique_ptr<DashboardCellBuilder> ValueDashboardCellBuilder::Clone() const {
  return std::make_unique<ValueDashboardCellBuilder>(m_value);
}
