#include "Spire/KeyBindings/SearchBarOrderTaskArgumentsListModel.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsMatch.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_filter(const QString& keywords, const CountryDatabase& countries,
      const MarketDatabase& markets, const DestinationDatabase& destinations) {
    auto identifiers =
      keywords.toLower().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    auto filter = [&, identifiers = std::move(identifiers)] (
        const auto& list, auto index) {
      if(identifiers.empty()) {
        return false;
      }
      auto& arguments = list.get(index);
      for(auto& identifier : identifiers) {
        if(matches(arguments, identifier, countries, markets, destinations)) {
          return false;
        }
      }
      return true;
    };
    return filter;
  }
}

SearchBarOrderTaskArgumentsListModel::SearchBarOrderTaskArgumentsListModel(
    std::shared_ptr<OrderTaskArgumentsListModel> source,
    std::shared_ptr<TextModel> keywords, CountryDatabase countries,
    MarketDatabase markets, DestinationDatabase destinations)
    : m_countries(std::move(countries)),
      m_markets(std::move(markets)),
      m_destinations(std::move(destinations)),
      m_filtered_list(std::move(source),
        make_filter(keywords->get(), m_countries, m_markets, m_destinations)),
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

void SearchBarOrderTaskArgumentsListModel::on_keywords(
    const QString& keywords) {
  m_filtered_list.set_filter(
    make_filter(keywords, m_countries, m_markets, m_destinations));
}
