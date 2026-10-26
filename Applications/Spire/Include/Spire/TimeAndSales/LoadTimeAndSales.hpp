#ifndef SPIRE_LOAD_TIME_AND_SALES_HPP
#define SPIRE_LOAD_TIME_AND_SALES_HPP
#include <vector>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /**
   * Loads a ticker's time and sales over a range, waiting until the query
   * completes.
   * @param client The client used to submit the query.
   * @param ticker The ticker whose time and sales are loaded.
   * @param range The range of time and sales to load.
   * @param limit The limit on the number of time and sales to load.
   * @return The time and sales loaded, in sequence order.
   */
  std::vector<Nexus::SequencedTimeAndSale> load_time_and_sales(
    Nexus::MarketDataClient& client, const Nexus::Ticker& ticker,
    const Beam::Range& range, const Beam::SnapshotLimit& limit);

  /**
   * Loads the BBO quote in effect at each entry and assigns the entry's
   * BboIndicator accordingly.
   * @param client The client used to load BBO quotes.
   * @param ticker The ticker the entries belong to.
   * @param entries The entries, in sequence order, to assign indicators to.
   * @param bbo The BBO quote in effect before the first entry, or none if
   *        unknown.
   * @return The BBO quote in effect at the last entry, or <i>bbo</i> if
   *         <i>entries</i> is empty.
   */
  boost::optional<Nexus::SequencedBboQuote> load_bbo_indicators(
    Nexus::MarketDataClient& client, const Nexus::Ticker& ticker,
    std::vector<TimeAndSalesModel::Type>& entries,
    boost::optional<Nexus::SequencedBboQuote> bbo);
}

#endif
