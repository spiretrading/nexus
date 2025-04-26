#include "Spire/KeyBindings/SearchBarOrderTaskArgumentsListModel.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SearchBarOrderTaskArgumentsListModel::SearchBarOrderTaskArgumentsListModel(
    std::shared_ptr<OrderTaskArgumentsListModel> source,
    std::shared_ptr<TextModel> keywords, CountryDatabase countries,
    MarketDatabase markets, DestinationDatabase destinations,
    AdditionalTagDatabase additional_tags)
    : m_cache(source, std::move(countries), std::move(markets),
        std::move(destinations), std::move(additional_tags)),
      m_filtered_list(std::move(source), make_filter(keywords->get())),
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
  return m_filtered_list.insert(value, index);
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

void SearchBarOrderTaskArgumentsListModel::transact(
    const std::function<void ()>& transaction) {
  m_filtered_list.transact([&] {
    transaction();
  });
}

FilteredListModel<OrderTaskArguments>::Filter
    SearchBarOrderTaskArgumentsListModel::make_filter(const QString& keywords) {
  return [=] (const auto& list, auto index) {
    if(keywords.isEmpty()) {
      return false;
    }
    return !matches(keywords, m_cache.get(index));
  };
}

void SearchBarOrderTaskArgumentsListModel::on_keywords(
    const QString& keywords) {
  m_filtered_list.set_filter(make_filter(keywords));
}
