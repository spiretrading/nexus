#include "Spire/Blotter/LocalValuationModel.hpp"
#include "Spire/Spire/ConstantValueModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Spire;

void LocalValuationModel::add(const Security& security,
    std::shared_ptr<CompositeModel<SecurityValuation>> valuation) {
  m_valuations.insert(std::pair(security, std::move(valuation)));
}

std::shared_ptr<CompositeModel<SecurityValuation>>
    LocalValuationModel::get_valuation(const Security& security) const {
  auto i = m_valuations.find(security);
  if(i == m_valuations.end()) {
    auto valuation = std::make_shared<CompositeModel<SecurityValuation>>();
    valuation->add(&SecurityValuation::m_currency,
      make_constant_value_model(DefaultCurrencies::USD()));
    valuation->add(&SecurityValuation::m_bidValue,
      make_constant_value_model(optional<Money>()));
    valuation->add(&SecurityValuation::m_askValue,
      make_constant_value_model(optional<Money>()));
    const_cast<LocalValuationModel*>(this)->add(security, valuation);
    return valuation;
  }
  return i->second;
}
