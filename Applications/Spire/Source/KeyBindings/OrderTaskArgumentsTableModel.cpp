#include "Spire/KeyBindings/OrderTaskArgumentsTableModel.hpp"
#include "Spire/Spire/ListToTableModel.hpp"

using namespace Nexus;
using namespace Spire;

namespace {
  static const auto COLUMN_SIZE = 9;

  AnyRef extract(const OrderTaskArguments& arguments, int index) {
    auto column = static_cast<OrderTaskColumns>(index);
    if(column == OrderTaskColumns::NAME) {
      return AnyRef(arguments.m_name);
    } else if(column == OrderTaskColumns::REGION) {
      return AnyRef(arguments.m_region);
    } else if(column == OrderTaskColumns::DESTINATION) {
      return AnyRef(arguments.m_destination);
    } else if(column == OrderTaskColumns::ORDER_TYPE) {
      return AnyRef(arguments.m_order_type);
    } else if(column == OrderTaskColumns::SIDE) {
      return AnyRef(arguments.m_side);
    } else if(column == OrderTaskColumns::QUANTITY) {
      return AnyRef(arguments.m_quantity);
    } else if(column == OrderTaskColumns::TIME_IN_FORCE) {
      return AnyRef(arguments.m_time_in_force);
    } else if(column == OrderTaskColumns::TAGS) {
      return AnyRef(arguments.m_additional_tags);
    } else {
      return AnyRef(arguments.m_key);
    }
  }
}

std::shared_ptr<TableModel> Spire::make_order_task_arguments_table_model(
    std::shared_ptr<OrderTaskArgumentsListModel> order_task_arguments) {
  return std::make_shared<ListToTableModel<OrderTaskArguments>>(
    std::move(order_task_arguments), COLUMN_SIZE, &extract);
}
