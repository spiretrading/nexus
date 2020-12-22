#include "Spire/Dashboard/ListDashboardRowBuilder.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"
#include "Spire/Dashboard/DashboardRow.hpp"
#include "Spire/Dashboard/ValueDashboardCell.hpp"

using namespace Beam;
using namespace Spire;
using namespace std;

ListDashboardRowBuilder::ListDashboardRowBuilder(
    vector<std::unique_ptr<DashboardCellBuilder>> cellBuilders)
    : m_cellBuilders(std::move(cellBuilders)) {}

ListDashboardRowBuilder::~ListDashboardRowBuilder() {}

std::unique_ptr<DashboardRow> ListDashboardRowBuilder::Make(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto indexCell = std::make_unique<ValueDashboardCell>();
  indexCell->SetBufferSize(1);
  indexCell->SetValue(index);
  auto row = std::make_unique<DashboardRow>(std::move(indexCell));
  for(auto& builder : m_cellBuilders) {
    auto cell = builder->Make(index, Ref(userProfile));
    row->Add(std::move(cell));
  }
  return row;
}

std::unique_ptr<DashboardRowBuilder> ListDashboardRowBuilder::Clone() const {
  vector<std::unique_ptr<DashboardCellBuilder>> cellBuilders;
  for(auto& cellBuilder : m_cellBuilders) {
    cellBuilders.push_back(cellBuilder->Clone());
  }
  return std::make_unique<ListDashboardRowBuilder>(std::move(cellBuilders));
}
