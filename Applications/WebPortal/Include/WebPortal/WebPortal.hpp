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
  using WebPosition = Accounting::Position<Nexus::Security>;
  using WebInventory = Accounting::Inventory<WebPosition>;
  using WebBookkeeper = Accounting::TrueAverageBookkeeper<WebInventory>;
  using WebPortfolio = Accounting::Portfolio<WebBookkeeper>;
}

#endif
