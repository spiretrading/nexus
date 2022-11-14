#include "Spire/KeyBindings/OrderTasksPage.hpp"

using namespace Nexus;
using namespace Spire;

OrderTasksPage::OrderTasksPage(
  std::shared_ptr<ComboBox::QueryModel> region_query_model,
  std::shared_ptr<ListModel<OrderTask>> model,
  const DestinationDatabase& destination_database,
  const MarketDatabase& market_database, QWidget* parent)
  : QWidget(parent),
    m_region_query_model(std::move(region_query_model)),
    m_list_model(std::move(model)),
    m_table(std::make_shared<OrderTaskTableModel>(m_list_model)) {}

const std::shared_ptr<ComboBox::QueryModel>&
    OrderTasksPage::get_region_query_model() const {
  return m_region_query_model;
}

const std::shared_ptr<ListModel<OrderTasksPage::OrderTask>>&
    OrderTasksPage::get_model() const {
  return m_list_model;
}
