#include "Spire/Spire/TickerInfoToTickerQueryModel.hpp"
#include <unordered_set>

using namespace boost;
using namespace Nexus;
using namespace Spire;

TickerInfoToTickerQueryModel::TickerInfoToTickerQueryModel(
  std::shared_ptr<TickerInfoQueryModel> source)
  : m_source(std::move(source)) {}

const std::shared_ptr<TickerInfoQueryModel>&
    TickerInfoToTickerQueryModel::get_source() const {
  return m_source;
}

optional<Ticker> TickerInfoToTickerQueryModel::parse(const QString& query) {
  if(auto value = m_source->parse(query.toUpper())) {
    return value->m_ticker;
  }
  return none;
}

QtPromise<std::vector<Ticker>>
    TickerInfoToTickerQueryModel::submit(const QString& query) {
  return m_source->submit(query.toUpper()).then(
    [] (const std::vector<TickerInfo>& matches) {
      auto seen = std::unordered_set<Ticker>();
      auto tickers = std::vector<Ticker>();
      for(auto& match : matches) {
        if(seen.insert(match.m_ticker).second) {
          tickers.push_back(match.m_ticker);
        }
      }
      return tickers;
    });
}
