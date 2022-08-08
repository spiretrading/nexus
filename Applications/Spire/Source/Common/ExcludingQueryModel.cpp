#include "Spire/Spire/ExcludingQueryModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

ExcludingQueryModel::ExcludingQueryModel(
    std::shared_ptr<ComboBox::QueryModel> source,
    std::shared_ptr<AnyListModel> exclusions)
    : m_source(std::move(source)),
      m_exclusions(std::move(exclusions)),
      m_connection(m_exclusions->connect_operation_signal(
        std::bind_front(&ExcludingQueryModel::on_operation, this))) {
  for(auto i = 0; i < m_exclusions->get_size(); ++i) {
    add_exclusion(i);
  }
}

std::any ExcludingQueryModel::parse(const QString& query) {
  auto value = m_source->parse(query);
  if(!value.has_value()) {
    return value;
  }
  if(m_exclusion_set.contains(to_string(value))) {
    return std::any();
  }
  return value;
}

QtPromise<std::vector<std::any>> ExcludingQueryModel::submit(
    const QString& query) {
  return m_source->submit(query).then([=] (auto&& source_result) {
    auto result = [&] {
      try {
        return source_result.Get();
      } catch(const std::exception&) {
        return std::vector<std::any>();
      }
    }();
    std::erase_if(result, [&] (auto& value) {
      return m_exclusion_set.contains(to_string(value));
    });
    return result;
  });
}

void ExcludingQueryModel::add_exclusion(int index) {
  auto value = displayText(m_exclusions->get(index));
  m_exclusion_set.insert(value);
  m_exclusion_list.insert(m_exclusion_list.begin() + index, value);
}

void ExcludingQueryModel::on_operation(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      add_exclusion(operation.m_index);
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      m_exclusion_set.erase(m_exclusion_list[operation.m_index]);
      m_exclusion_list.erase(m_exclusion_list.begin() + operation.m_index);
    });
}

QString ExcludingQueryModel::to_string(const std::any& value) {
  return displayText(value);
}
