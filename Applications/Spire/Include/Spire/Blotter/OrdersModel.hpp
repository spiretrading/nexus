#ifndef SPIRE_ORDERS_MODEL_HPP
#define SPIRE_ORDERS_MODEL_HPP
#include <memory>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** An observable list of orders. */
  using OrdersModel = ListModel<std::shared_ptr<Nexus::Order>>;
}

#endif
