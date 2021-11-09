#include "Spire/Ui/LocalComboBoxQueryModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Beam::Queries;
using namespace Spire;
using Query = ComboBox::QueryModel::Query;

void LocalComboBoxQueryModel::add(const std::any& value) {
  if(index_of(value)) {
    return;
  }
  m_data.push_back(value);
}
  
void LocalComboBoxQueryModel::remove(const std::any& value) {
  if(auto index = index_of(value)) {
    m_data.erase(m_data.begin() + *index);
  }
}

QtPromise<std::vector<std::any>>
    LocalComboBoxQueryModel::query(const Query& query) const {
  if(query.m_limit == SnapshotLimit::None()) {
    return QtPromise(std::vector<std::any>());
  }
  // TODO: reentrancy, and what happens if the data is modified by a query?
  auto data = std::vector<std::any>();
  for(auto& item : m_data) {
    if(displayTextAny(item).toLower().startsWith(
        QString::fromStdString(query.m_text).toLower())) {
      data.push_back(item);
      if(data.size() == query.m_limit.GetSize()) {
        break;
      }
    }
  }
  return QtPromise(data);
}

boost::optional<size_t>
    LocalComboBoxQueryModel::index_of(const std::any& value) {
  auto index = boost::optional<size_t>();
  auto value_text = displayTextAny(value);
  auto iter = std::find_if(m_data.begin(), m_data.end(),
    [&] (const auto& item) { return value_text == displayTextAny(item); });
  if(iter != m_data.end()) {
    index = std::distance(m_data.begin(), iter); 
  }
  return index;
}
