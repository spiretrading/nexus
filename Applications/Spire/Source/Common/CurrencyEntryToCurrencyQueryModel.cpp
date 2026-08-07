#include "Spire/Spire/CurrencyEntryToCurrencyQueryModel.hpp"
#include <unordered_set>

using namespace boost;
using namespace Nexus;
using namespace Spire;

CurrencyEntryToCurrencyQueryModel::CurrencyEntryToCurrencyQueryModel(
  std::shared_ptr<CurrencyEntryQueryModel> source)
  : m_source(std::move(source)) {}

const std::shared_ptr<CurrencyEntryQueryModel>&
    CurrencyEntryToCurrencyQueryModel::get_source() const {
  return m_source;
}

optional<CurrencyId>
    CurrencyEntryToCurrencyQueryModel::parse(const QString& query) {
  if(auto entry = m_source->parse(query.toUpper())) {
    return entry->m_id;
  }
  return none;
}

QtPromise<std::vector<CurrencyId>>
    CurrencyEntryToCurrencyQueryModel::submit(const QString& query) {
  return m_source->submit(query.toUpper()).then(
    [] (const std::vector<CurrencyDatabase::Entry>& matches) {
      auto seen = std::unordered_set<CurrencyId>();
      auto ids = std::vector<CurrencyId>();
      for(auto& match : matches) {
        if(seen.insert(match.m_id).second) {
          ids.push_back(match.m_id);
        }
      }
      return ids;
    });
}
