#ifndef SPIRE_HPP
#define SPIRE_HPP
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal_type.hpp>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/MarketDataService/MarketDataClientBox.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#ifdef _DEBUG
  #include <QDebug>
#endif

namespace Spire {
  class SpireServiceClients;
  using SpirePosition = Nexus::Accounting::Position<Nexus::Security>;
  using SpireInventory = Nexus::Accounting::Inventory<SpirePosition>;
  using SpireBookkeeper = Nexus::Accounting::TrueAverageBookkeeper<
    SpireInventory>;
  using SpirePortfolio = Nexus::Accounting::Portfolio<SpireBookkeeper>;
  using SpirePortfolioController = Nexus::Accounting::PortfolioController<
    SpirePortfolio, Nexus::MarketDataService::MarketDataClientBox>;
  class UserProfile;

  /** Defines the common type of boost signal used throughout Spire. */
  template<typename F>
  using Signal = typename boost::signals2::signal_type<F,
    boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>>::type;
}

#endif
