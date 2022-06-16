#ifndef SPIRE_VALUATION_MODEL_HPP
#define SPIRE_VALUATION_MODEL_HPP
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/CompositeModel.hpp"

namespace Spire {

  /** Stores the valuation of a single security. */
  struct Valuation {

    /** The valuation on the bid. */
    Nexus::Money m_bid;

    /** The valuation on the ask. */
    Nexus::Money m_ask;
  };

  /** Interface for loading security valuation models. */
  class ValuationModel {
    public:
      virtual ~ValuationModel() = default;

      /**
       * Returns a model representing a security's valuation.
       * @param security The security whose valuation model is returned.
       */
      virtual std::shared_ptr<CompositeModel<Valuation>> get_valuation(
        const Nexus::Security& security) const = 0;
  };
}

#endif
