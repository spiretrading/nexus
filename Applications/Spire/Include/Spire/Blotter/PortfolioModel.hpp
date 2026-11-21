#ifndef SPIRE_PORTFOLIO_MODEL_HPP
#define SPIRE_PORTFOLIO_MODEL_HPP
#include "Nexus/Accounting/Portfolio.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** An observable list of portfolio entries. */
  using PortfolioModel = ListModel<Nexus::PortfolioUpdateEntry>;
}

#endif
