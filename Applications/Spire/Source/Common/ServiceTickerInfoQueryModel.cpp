#include "Spire/Spire/ServiceTickerInfoQueryModel.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

ServiceTickerInfoQueryModel::ServiceTickerInfoQueryModel(
  MarketDataClient market_data_client)
  : m_market_data_client(std::move(market_data_client)) {}

optional<TickerInfo> ServiceTickerInfoQueryModel::parse(const QString& query) {
  auto ticker = parse_ticker(query.toStdString());
  if(!ticker) {
    return none;
  }
  if(auto info = m_info.try_load(ticker)) {
    return *info;
  }
  auto info = TickerInfo();
  info.m_ticker = std::move(ticker);
  return info;
}

QtPromise<std::vector<TickerInfo>>
    ServiceTickerInfoQueryModel::submit(const QString& query) {
  if(query.isEmpty()) {
    return {};
  }
  return QtPromise([=, client = m_market_data_client] () mutable {
    auto tickers =
      client.load_ticker_info_from_prefix(query.trimmed().toStdString());
    for(auto& ticker : tickers) {
      m_info.insert(ticker.m_ticker, ticker);
    }
    return tickers;
  }, LaunchPolicy::ASYNC);
}
