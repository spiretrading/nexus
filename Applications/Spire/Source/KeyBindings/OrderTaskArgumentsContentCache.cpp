#include "Spire/KeyBindings/OrderTaskArgumentsContentCache.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  auto split(const QString& name) {
    return name.split(' ', Qt::SkipEmptyParts);
  }
}

OrderTaskArgumentsContentCache::OrderTaskArgumentsContentCache(
  std::shared_ptr<OrderTaskArgumentsListModel> model,
  AdditionalTagDatabase additional_tags)
  : m_model(std::move(model)),
    m_additional_tags(std::move(additional_tags)),
    m_connection(m_model->connect_operation_signal(
      std::bind_front(&OrderTaskArgumentsContentCache::on_operation, this))) {}

const std::vector<QString>& OrderTaskArgumentsContentCache::get(int index) {
  auto i = m_cache.find(index);
  if(i != m_cache.end()) {
    return i->second;
  }
  auto row_words = std::vector<QString>();
  auto& arguments = m_model->get(index);
  for(auto& name : split(arguments.m_name)) {
    row_words.push_back(name);
  }
  for(auto& region : ::to_text(arguments.m_region)) {
    row_words.push_back(region);
  }
  row_words.push_back(QString::fromStdString(
    DEFAULT_DESTINATIONS.from(arguments.m_destination).m_id));
  row_words.push_back(to_text(arguments.m_order_type));
  row_words.push_back(to_text(arguments.m_side));
  row_words.push_back(to_text(arguments.m_quantity));
  row_words.push_back(to_text(arguments.m_time_in_force));
  for(auto& tag : arguments.m_additional_tags) {
    auto schema = find(m_additional_tags, arguments.m_destination,
      arguments.m_region, tag.m_key);
    row_words.push_back(QString::fromStdString(schema->get_name()));
  }
  for(auto& name : split(arguments.m_key.toString().replace('+', ' '))) {
    row_words.push_back(name);
  }
  return m_cache.emplace(index, std::move(row_words)).first->second;
}

void OrderTaskArgumentsContentCache::on_operation(
    const OrderTaskArgumentsListModel::Operation& operation) {
  visit(operation,
    [&] (const OrderTaskArgumentsListModel::MoveOperation& operation) {
      auto [min, max] =
        std::minmax(operation.m_source, operation.m_destination);
      for(auto i = min; i <= max; ++i) {
        if(m_cache.find(i) != m_cache.end()) {
          m_cache.erase(i);
        }
      }
    },
    [&] (const OrderTaskArgumentsListModel::PreRemoveOperation& operation) {
      if(m_cache.find(operation.m_index) != m_cache.end()) {
        m_cache.erase(operation.m_index);
      }
    },
    [&] (const OrderTaskArgumentsListModel::UpdateOperation& operation) {
      if(m_cache.find(operation.m_index) != m_cache.end()) {
        m_cache.erase(operation.m_index);
      }
  });
}
