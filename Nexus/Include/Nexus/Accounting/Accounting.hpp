#ifndef NEXUS_ACCOUNTING_HPP
#define NEXUS_ACCOUNTING_HPP

namespace Nexus::Accounting {
  template<typename InventoryType> class Bookkeeper;
  class BuyingPowerTracker;
  template<typename PositionType> struct Inventory;
  template<typename BookkeeperType> class Portfolio;
  template<typename PortfolioType, typename MarketDataClientType>
    class PortfolioMonitor;
  template<typename InventoryType> struct PortfolioUpdateEntry;
  template<typename IndexType> struct Position;
  class PositionOrderBook;
  struct SecurityValuation;
  class ShortingTracker;
  template<typename InventoryType> class TrueAverageBookkeeper;
}

#endif
