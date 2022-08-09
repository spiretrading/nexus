#include "Spire/Spire/ExcludingQueryModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

ExcludingQueryModel::ExcludingQueryModel(
  std::shared_ptr<ComboBox::QueryModel> source,
  std::shared_ptr<AnyListModel> exclusions)
  : m_source(std::move(source)),
    m_exclusions(std::move(exclusions)) {}

std::any ExcludingQueryModel::parse(const QString& query) {
  auto value = m_source->parse(query);
  if(!value.has_value()) {
    return value;
  }
  if(is_in_exclusion(value)) {
    static auto empty_value = std::any();
    return empty_value;
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
        static auto empty_matches = std::vector<std::any>();
        return empty_matches;
      }
    }();
    std::erase_if(result, [&] (auto& value) {
      return is_in_exclusion(value);
    });
    return result;
  });
}

bool ExcludingQueryModel::is_in_exclusion(const std::any& value) {
  for(auto i = 0; i < m_exclusions->get_size(); ++i) {
    if(is_equal(value, m_source->parse(displayText(m_exclusions->get(i))))) {
      return true;
    }
  }
  return false;
}
