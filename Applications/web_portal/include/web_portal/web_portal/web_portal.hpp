#ifndef NEXUS_CLIENTWEBPORTAL_HPP
#define NEXUS_CLIENTWEBPORTAL_HPP
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

namespace Nexus {
namespace ClientWebPortal {
  using WebPosition = Nexus::Accounting::Position<Nexus::Security>;
  using WebInventory = Nexus::Accounting::Inventory<WebPosition>;
  using WebBookkeeper = Nexus::Accounting::TrueAverageBookkeeper<WebInventory>;
  using WebPortfolio = Nexus::Accounting::Portfolio<WebBookkeeper>;
  class AdministrationWebServlet;
  class ClientWebPortalServlet;
  class ClientWebPortalSession;
  class ComplianceWebServlet;
  class DefinitionsWebServlet;
  class MarketDataWebServlet;
  class RiskWebServlet;
  class ServiceLocatorWebServlet;
  class PortfolioModel;
}
}

#endif
