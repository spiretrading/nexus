#ifndef NEXUS_ACCOUNTING_HPP
#define NEXUS_ACCOUNTING_HPP

namespace Nexus::Accounting {
  template<typename I> class Bookkeeper;
  class BuyingPowerModel;
  template<typename P> struct Inventory;
  template<typename B> class Portfolio;
  template<typename P, typename C> class PortfolioController;
  template<typename I> struct PortfolioUpdateEntry;
  template<typename I> struct Position;
  class PositionOrderBook;
  struct SecurityValuation;
  class ShortingModel;
  template<typename I> class TrueAverageBookkeeper;
}

#endif
