#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/Dashboard/BboPriceDashboardCellBuilder.hpp"
#include "Spire/Dashboard/BboSizeDashboardCellBuilder.hpp"
#include "Spire/Dashboard/ChangeDashboardCellBuilder.hpp"
#include "Spire/Dashboard/CloseDashboardCellBuilder.hpp"
#include "Spire/Dashboard/DashboardModel.hpp"
#include "Spire/Dashboard/DashboardRowBuilder.hpp"
#include "Spire/Dashboard/HighDashboardCellBuilder.hpp"
#include "Spire/Dashboard/LastPriceDashboardCellBuilder.hpp"
#include "Spire/Dashboard/ListDashboardRowBuilder.hpp"
#include "Spire/Dashboard/LowDashboardCellBuilder.hpp"
#include "Spire/Dashboard/ValueDashboardCellBuilder.hpp"
#include "Spire/Dashboard/VolumeDashboardCellBuilder.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

DashboardModelSchema DashboardModelSchema::GetDefaultSchema() {
  vector<string> columns = {"Security", "Close", "Last", "Change", "Bid Px",
    "Bid Sz", "Ask Sz", "Ask Px", "High", "Low", "Volume"};
  vector<std::unique_ptr<DashboardCellBuilder>> cellBuilders;
  cellBuilders.push_back(std::make_unique<CloseDashboardCellBuilder>());
  cellBuilders.push_back(std::make_unique<LastPriceDashboardCellBuilder>());
  cellBuilders.push_back(std::make_unique<ChangeDashboardCellBuilder>());
  cellBuilders.push_back(
    std::make_unique<BboPriceDashboardCellBuilder>(Side::BID));
  cellBuilders.push_back(
    std::make_unique<BboSizeDashboardCellBuilder>(Side::BID));
  cellBuilders.push_back(
    std::make_unique<BboSizeDashboardCellBuilder>(Side::ASK));
  cellBuilders.push_back(
    std::make_unique<BboPriceDashboardCellBuilder>(Side::ASK));
  cellBuilders.push_back(std::make_unique<HighDashboardCellBuilder>());
  cellBuilders.push_back(std::make_unique<LowDashboardCellBuilder>());
  cellBuilders.push_back(std::make_unique<VolumeDashboardCellBuilder>());
  ListDashboardRowBuilder rowBuilder{std::move(cellBuilders)};
  DashboardModelSchema schema{std::move(columns),
    vector<DashboardCell::Value>(), rowBuilder};
  return schema;
}

DashboardModelSchema::DashboardModelSchema() {
  m_columnNames.push_back("Security");
  vector<std::unique_ptr<DashboardCellBuilder>> cellBuilders;
  cellBuilders.push_back(
    std::make_unique<ValueDashboardCellBuilder>(Security{}));
  m_rowBuilder = make_unique<ListDashboardRowBuilder>(std::move(cellBuilders));
}

DashboardModelSchema::DashboardModelSchema(vector<string> columnNames,
    vector<DashboardCell::Value> rowIndices,
    const DashboardRowBuilder& rowBuilder)
    : m_columnNames(std::move(columnNames)),
      m_rowIndices(std::move(rowIndices)),
      m_rowBuilder(rowBuilder.Clone()) {}

DashboardModelSchema::DashboardModelSchema(const DashboardModel& model,
    const DashboardRowBuilder& rowBuilder) {
  for(auto i = 0; i < model.GetColumnCount(); ++i) {
    m_columnNames.push_back(model.GetColumnName(i));
  }
  for(auto i = 0; i < model.GetRowCount(); ++i) {
    m_rowIndices.push_back(model.GetRow(i).GetIndex().GetValues().back());
  }
  m_rowBuilder = rowBuilder.Clone();
}

DashboardModelSchema::DashboardModelSchema(const DashboardModelSchema& schema)
    : m_columnNames(schema.m_columnNames),
      m_rowIndices(schema.m_rowIndices),
      m_rowBuilder(schema.GetRowBuilder().Clone()) {}

DashboardModelSchema::~DashboardModelSchema() {}

DashboardModelSchema& DashboardModelSchema::operator =(
    const DashboardModelSchema& schema) {
  m_columnNames = schema.m_columnNames;
  m_rowIndices = schema.m_rowIndices;
  m_rowBuilder = schema.GetRowBuilder().Clone();
  return *this;
}

const vector<string>& DashboardModelSchema::GetColumnNames() const {
  return m_columnNames;
}

const vector<DashboardCell::Value>& DashboardModelSchema::
    GetRowIndices() const {
  return m_rowIndices;
}

const DashboardRowBuilder& DashboardModelSchema::GetRowBuilder() const {
  return *m_rowBuilder;
}

std::unique_ptr<DashboardModel> DashboardModelSchema::Make(
    Ref<UserProfile> userProfile) const {
  auto model = std::make_unique<DashboardModel>(m_columnNames);
  for(auto& index : m_rowIndices) {
    auto row = GetRowBuilder().Make(index, Ref(userProfile));
    model->Add(std::move(row));
  }
  return model;
}
