#ifndef NEXUS_WEB_PORTAL_HPP
#define NEXUS_WEB_PORTAL_HPP
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"

namespace Nexus::WebPortal {
  class AdministrationWebServlet;
  class ComplianceWebServlet;
  class DefinitionsWebServlet;
  class MarketDataWebServlet;
  class RiskWebServlet;
  class ServiceLocatorWebServlet;
  class PortfolioModel;
  class WebPortalServlet;
  class WebPortalSession;
  using WebPosition = Nexus::Accounting::Position<Nexus::Security>;
  using WebInventory = Nexus::Accounting::Inventory<WebPosition>;
  using WebBookkeeper = Nexus::Accounting::TrueAverageBookkeeper<WebInventory>;
  using WebPortfolio = Nexus::Accounting::Portfolio<WebBookkeeper>;
}

#endif
