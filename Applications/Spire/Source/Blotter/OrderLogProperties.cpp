#include "Spire/Blotter/OrderLogProperties.hpp"

using namespace Spire;

OrderLogProperties OrderLogProperties::GetDefault() {
  OrderLogProperties properties;
  properties.m_orderStatusFilterType = OrderStatusFilterType::LIVE_ORDERS;
  return properties;
}
