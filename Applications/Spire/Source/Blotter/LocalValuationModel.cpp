#include "Spire/Blotter/LocalValuationModel.hpp"

using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Spire;

std::shared_ptr<CompositeModel<SecurityValuation>>
    LocalValuationModel::get_valuation(const Security& security) const {
  return nullptr;
}
