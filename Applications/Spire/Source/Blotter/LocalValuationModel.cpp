#include "Spire/Blotter/LocalValuationModel.hpp"

using namespace Spire;

std::shared_ptr<CompositeModel<Valuation>> LocalValuationModel::get_valuation(
    const Nexus::Security& security) const {
  return nullptr;
}
