#include "Spire/Canvas/Operations/Translation.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

const std::type_info& Translation::GetTypeInfo() const {
  return typeid(int);
}

const Publisher<const Order*>* Translation::GetPublisher() const {
  return m_publisher;
}
