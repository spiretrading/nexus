#ifndef SPIRE_UNSIGNED_QUANTITY_MODEL_HPP
#define SPIRE_UNSIGNED_QUANTITY_MODEL_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Spire/LocalScalarValueModel.hpp"

namespace Spire {

  /** Represents a model for unsigned Quantity values. */
  struct UnsignedQuantityModel :
      ScalarValueModel<boost::optional<Nexus::Quantity>> {
    public:

      /**
       * Constructs an UnsignedQuantityModel.
       * @param model The model of the signed Quantity.
       */
      explicit UnsignedQuantityModel(std::shared_ptr<
        ScalarValueModel<boost::optional<Nexus::Quantity>>> model);

      boost::optional<Nexus::Quantity> get_minimum() const override;

      boost::optional<Nexus::Quantity> get_maximum() const override;

      Nexus::Quantity get_increment() const override;

      QValidator::State get_state() const override;

      const boost::optional<Nexus::Quantity>& get_current() const override;

      QValidator::State set_current(
        const boost::optional<Nexus::Quantity>& value) override;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ScalarValueModel<boost::optional<Nexus::Quantity>>> m_model;
  };
}

#endif
