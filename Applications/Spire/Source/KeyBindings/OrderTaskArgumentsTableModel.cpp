#include "Spire/KeyBindings/OrderTaskArgumentsTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  static const auto COLUMN_SIZE = 9;

  AnyRef extract(OrderTaskArguments& arguments, int index) {
    auto column = static_cast<OrderTaskColumns>(index);
    if(column == OrderTaskColumns::NAME) {
      return arguments.m_name;
    } else if(column == OrderTaskColumns::REGION) {
      return arguments.m_region;
    } else if(column == OrderTaskColumns::DESTINATION) {
      return arguments.m_destination;
    } else if(column == OrderTaskColumns::ORDER_TYPE) {
      return arguments.m_order_type;
    } else if(column == OrderTaskColumns::SIDE) {
      return arguments.m_side;
    } else if(column == OrderTaskColumns::QUANTITY) {
      return arguments.m_quantity;
    } else if(column == OrderTaskColumns::TIME_IN_FORCE) {
      return arguments.m_time_in_force;
    } else if(column == OrderTaskColumns::TAGS) {
      return arguments.m_additional_tags;
    } else {
      return arguments.m_key;
    }
  }
}

std::shared_ptr<TableModel> Spire::make_order_task_arguments_table_model(
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments) {
  return std::make_shared<ListToTableModel<OrderTaskArguments>>(
    std::move(order_task_arguments), COLUMN_SIZE, &extract);
}
