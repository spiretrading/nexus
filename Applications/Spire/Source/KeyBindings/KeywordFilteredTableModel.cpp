#include "Spire/KeyBindings/KeywordFilteredTableModel.hpp"
#include "Spire/KeyBindings/OrderTaskArgumentsListToTableModel.hpp"
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
        const auto& table, auto index) {
      if(identifiers.empty()) {
        return false;
      }
      auto extract = [&] <typename T> (OrderTaskColumns column) {
        return table.get<T>(index, static_cast<int>(column));
      };
      for(auto& identifier : identifiers) {
        if(matches(
            extract.operator ()<QString>(OrderTaskColumns::NAME), identifier)) {
          return false;
        } else if(matches(extract.operator ()<Region>(OrderTaskColumns::REGION),
            identifier, countries, markets)) {
          return false;
        } else if(matches(
            extract.operator ()<Destination>(OrderTaskColumns::DESTINATION),
            identifier, destinations)) {
          return false;
        } else if(matches(
            extract.operator ()<OrderType>(OrderTaskColumns::ORDER_TYPE),
            identifier)) {
          return false;
        } else if(matches(
            extract.operator ()<Side>(OrderTaskColumns::SIDE), identifier)) {
          return false;
        } else if(matches(
            extract.operator ()<QuantitySetting>(OrderTaskColumns::QUANTITY),
            identifier)) {
          return false;
        } else if(matches(
            extract.operator ()<TimeInForce>(OrderTaskColumns::TIME_IN_FORCE),
            identifier)) {
          return false;
        } else if(matches(
            extract.operator ()<QKeySequence>(OrderTaskColumns::KEY),
            identifier)) {
          return false;
        }
      }
      return true;
    };
    return filter;
  }
}

KeywordFilteredTableModel::KeywordFilteredTableModel(
    std::shared_ptr<TableModel> source, std::shared_ptr<TextModel> keywords,
    const CountryDatabase& countries, const MarketDatabase& markets,
    const DestinationDatabase& destinations)
    : m_countries(std::move(countries)),
      m_markets(std::move(markets)),
      m_destinations(std::move(destinations)),
      m_filtered_model(std::move(source),
        make_filter(keywords->get(), m_countries, m_markets, m_destinations)),
      m_keywords(std::move(keywords)) {
  m_connection = m_keywords->connect_update_signal(
    std::bind_front(&KeywordFilteredTableModel::on_keywords, this));
}

int KeywordFilteredTableModel::get_row_size() const {
  return m_filtered_model.get_row_size();
}

int KeywordFilteredTableModel::get_column_size() const {
  return m_filtered_model.get_column_size();
}

AnyRef KeywordFilteredTableModel::at(int row, int column) const {
  return m_filtered_model.at(row, column);
}

QValidator::State KeywordFilteredTableModel::set(
    int row, int column, const std::any& value) {
  return m_filtered_model.set(row, column, value);
}

QValidator::State KeywordFilteredTableModel::remove(int row) {
  return m_filtered_model.remove(row);
}

connection KeywordFilteredTableModel::connect_operation_signal(
    const OperationSignal::slot_type& slot) const {
  return m_filtered_model.connect_operation_signal(slot);
}

void KeywordFilteredTableModel::on_keywords(const QString& keywords) {
  m_filtered_model.set_filter(
    make_filter(keywords, m_countries, m_markets, m_destinations));
}
