#include "Spire/KeyBindings/SearchBarOrderTaskArgumentsListModel.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SearchBarOrderTaskArgumentsListModel::SearchBarOrderTaskArgumentsListModel(
    std::shared_ptr<OrderTaskArgumentsListModel> source,
    std::shared_ptr<TextModel> keywords, AdditionalTagDatabase additional_tags)
    : m_source(std::move(source)),
      m_cache(m_source, std::move(additional_tags)),
      m_filtered_list(m_source, make_filter(keywords->get())),
      m_keywords(std::move(keywords)) {
  m_connection = m_keywords->connect_update_signal(
    std::bind_front(&SearchBarOrderTaskArgumentsListModel::on_keywords, this));
}

int SearchBarOrderTaskArgumentsListModel::get_size() const {
  return m_filtered_list.get_size();
}

const OrderTaskArguments&
    SearchBarOrderTaskArgumentsListModel::get(int index) const {
  return m_filtered_list.get(index);
}

QValidator::State SearchBarOrderTaskArgumentsListModel::set(
    int index, const OrderTaskArguments& value) {
  return m_filtered_list.set(index, value);
}

QValidator::State SearchBarOrderTaskArgumentsListModel::push(
    const OrderTaskArguments& value) {
  return m_filtered_list.push(value);
}

QValidator::State SearchBarOrderTaskArgumentsListModel::insert(
    const OrderTaskArguments& value, int index) {
  if(index == get_size()) {
    return m_source->push(value);
  }
  return m_source->insert(value, m_filtered_list.index_to_source(index));
}

QValidator::State SearchBarOrderTaskArgumentsListModel::move(
    int source, int destination) {
  return m_filtered_list.move(source, destination);
}

QValidator::State SearchBarOrderTaskArgumentsListModel::remove(int index) {
  return m_filtered_list.remove(index);
}

connection SearchBarOrderTaskArgumentsListModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_filtered_list.connect_operation_signal(slot);
}

void SearchBarOrderTaskArgumentsListModel::start_transaction() {
  m_filtered_list.start_transaction();
}

void SearchBarOrderTaskArgumentsListModel::end_transaction() {
  m_filtered_list.end_transaction();
}

FilteredListModel<OrderTaskArguments>::Filter
    SearchBarOrderTaskArgumentsListModel::make_filter(const QString& keywords) {
  auto query_words = keywords.toLower().split(' ', Qt::SkipEmptyParts);
  return [this, query_words = std::move(query_words)] (
      const auto& list, auto index) {
    if(query_words.isEmpty()) {
      return false;
    }
    return !matches(query_words, m_cache.get(index));
  };
}

void SearchBarOrderTaskArgumentsListModel::on_keywords(
    const QString& keywords) {
  m_filtered_list.set_filter(make_filter(keywords));
}
