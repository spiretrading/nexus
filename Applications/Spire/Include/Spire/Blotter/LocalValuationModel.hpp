#ifndef SPIRE_LOCAL_VALUATION_MODEL_HPP
#define SPIRE_LOCAL_VALUATION_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/ValuationModel.hpp"

namespace Spire {

  /** Implements a ValuationModel using local memory. */
  class LocalValuationModel : public ValuationModel {
    public:
      std::shared_ptr<CompositeModel<Nexus::Accounting::SecurityValuation>>
        get_valuation(const Nexus::Security& security) const override;
  };
}

#endif
