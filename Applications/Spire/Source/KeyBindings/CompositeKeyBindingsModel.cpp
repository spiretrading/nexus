#include "Spire/KeyBindings/CompositeKeyBindingsModel.hpp"

using namespace Nexus;
using namespace Spire;

CompositeKeyBindingsModel::CompositeKeyBindingsModel(
  std::shared_ptr<ListModel<OrderTask>> order_tasks,
  std::shared_ptr<ListModel<OrderTask>> default_order_tasks,
  std::shared_ptr<ComboBox::QueryModel> region_query_model,
  DestinationDatabase destinations, MarketDatabase markets)
  : m_order_tasks(std::move(order_tasks)),
    m_default_order_tasks(std::move(default_order_tasks)),
    m_region_query_model(std::move(region_query_model)),
    m_destinations(std::move(destinations)),
    m_markets(std::move(markets)) {}

std::shared_ptr<ListModel<OrderTask>>
    CompositeKeyBindingsModel::get_order_tasks() {
  return m_order_tasks;
}

std::shared_ptr<ListModel<OrderTask>>
    CompositeKeyBindingsModel::get_default_order_tasks() const {
  return m_default_order_tasks;
}

std::shared_ptr<ComboBox::QueryModel>
    CompositeKeyBindingsModel::get_region_query_model() {
  return m_region_query_model;
}

const DestinationDatabase& CompositeKeyBindingsModel::get_destinations() const {
  return m_destinations;
}

const MarketDatabase& CompositeKeyBindingsModel::get_markets() const {
  return m_markets;
}
