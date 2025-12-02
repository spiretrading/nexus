#ifndef SPIRE_UNSIGNED_QUANTITY_MODEL_HPP
#define SPIRE_UNSIGNED_QUANTITY_MODEL_HPP
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Spire/ScalarValueModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Represents a model for unsigned Quantity values. */
  class UnsignedQuantityModel :
      public ScalarValueModel<boost::optional<Nexus::Quantity>> {
    public:

      /**
       * Constructs an UnsignedQuantityModel.
       * @param model The model of the signed Quantity.
       */
      explicit UnsignedQuantityModel(std::shared_ptr<
        ScalarValueModel<boost::optional<Nexus::Quantity>>> model);

      boost::optional<Nexus::Quantity> get_minimum() const override;
      boost::optional<Nexus::Quantity> get_maximum() const override;
      boost::optional<Nexus::Quantity> get_increment() const override;
      QValidator::State get_state() const override;
      const boost::optional<Nexus::Quantity>& get() const override;
      QValidator::State test(
        const boost::optional<Nexus::Quantity>& value) const override;
      QValidator::State set(
        const boost::optional<Nexus::Quantity>& value) override;
      boost::signals2::connection connect_update_signal(
        const typename UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<ScalarValueModel<boost::optional<Nexus::Quantity>>>
        m_model;
  };
}

#endif
