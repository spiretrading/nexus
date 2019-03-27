#ifndef SPIRE_LOCAL_TECHNICALS_MODEL_HPP
#define SPIRE_LOCAL_TECHNICALS_MODEL_HPP
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "spire/spire/technicals_model.hpp"

namespace Spire {

  /** Implements a TechnicalsModel in memory. */
  class LocalTechnicalsModel : public TechnicalsModel {
    public:

      //! Constructs a LocalTechnicalsModel.
      /*!
        \param security The security to model.
      */
      LocalTechnicalsModel(Nexus::Security security);

      //! Sets the closing price.
      /*!
        \param price The closing price to use.
      */
      void set_close(Nexus::Money price);

      //! Updates the model with a TimeAndSale.
      /*!
        \param time_and_sale The TimeAndSale containing a trade used to update
               the technicals, ie. price and quantity.
      */
      void update(const Nexus::TimeAndSale& time_and_sale);

      const Nexus::Security& get_security() const override;

      boost::optional<Nexus::Money> get_high() const override;

      boost::optional<Nexus::Money> get_low() const override;

      boost::optional<Nexus::Money> get_open() const override;

      boost::optional<Nexus::Money> get_close() const override;

      boost::optional<Nexus::Money> get_last_price() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<void> load() override;

      boost::signals2::connection connect_high_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_low_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_open_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_close_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_last_price_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_slot(
        const QuantitySignal::slot_type& slot) const override;
  };
}

#endif
