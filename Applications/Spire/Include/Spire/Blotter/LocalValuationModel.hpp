#ifndef SPIRE_LOCAL_VALUATION_MODEL_HPP
#define SPIRE_LOCAL_VALUATION_MODEL_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/ValuationModel.hpp"

namespace Spire {

  /**
   * Implements a ValuationModel where valuations are programmatically added.
   * When retrieiving a valuation that has not yet been added, a model is
   * returned whose valuation and currency are set to none.
   */
  class LocalValuationModel : public ValuationModel {
    public:

      /**
       * Adds a valuation to this model that can later be retrieved by calling
       * <i>get_valuation()</i>.
       * @param security The security to add the valuation for.
       * @param valuation The valuation to associate with the <i>security</i>.
       */
      void add(const Nexus::Security& security, std::shared_ptr<
        CompositeModel<Nexus::Accounting::SecurityValuation>> valuation);

      std::shared_ptr<CompositeModel<Nexus::Accounting::SecurityValuation>>
        get_valuation(const Nexus::Security& security) const override;
  };
}

#endif
