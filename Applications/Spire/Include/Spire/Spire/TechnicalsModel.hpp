#ifndef SPIRE_TECHNICALS_MODEL_HPP
#define SPIRE_TECHNICALS_MODEL_HPP
#include <memory>
#include <boost/optional.hpp>
#include <boost/signals2.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Async/QtPromise.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Models a security's technical indicators. */
  class TechnicalsModel {
    public:

      /** Signals a price update. */
      using PriceSignal = Signal<void (Nexus::Money value)>;

      /** Signals a quantity update. */
      using QuantitySignal = Signal<void (Nexus::Quantity value)>;

      virtual ~TechnicalsModel() = default;

      /** Returns the security being modeled. */
      virtual const Nexus::Security& get_security() const = 0;

      /** Returns the highest price of the session. */
      virtual boost::optional<Nexus::Money> get_high() const = 0;

      /** Returns the lowest price of the session. */
      virtual boost::optional<Nexus::Money> get_low() const = 0;

      /** Returns the session's opening price. */
      virtual boost::optional<Nexus::Money> get_open() const = 0;

      /** Returns the previous session's closing price. */
      virtual boost::optional<Nexus::Money> get_close() const = 0;

      /** Returns the price of the last trade. */
      virtual boost::optional<Nexus::Money> get_last_price() const = 0;

      /** Returns the session's volume. */
      virtual Nexus::Quantity get_volume() const = 0;

      /** Loads the model. */
      virtual QtPromise<void> load() = 0;

      /**
       * Connects a slot to the high signal.
       * @param slot The slot to connect.
       */
      virtual boost::signals2::connection connect_high_slot(
        const PriceSignal::slot_type& slot) const = 0;

      /**
       * Connects a slot to the low signal.
       * @param slot The slot to connect.
       */
      virtual boost::signals2::connection connect_low_slot(
        const PriceSignal::slot_type& slot) const = 0;

      /**
       * Connects a slot to the open signal.
       * @param slot The slot to connect.
       */
      virtual boost::signals2::connection connect_open_slot(
        const PriceSignal::slot_type& slot) const = 0;

      /**
       * Connects a slot to the close signal.
       * @param slot The slot to connect.
       */
      virtual boost::signals2::connection connect_close_slot(
        const PriceSignal::slot_type& slot) const = 0;

      /**
       * Connects a slot to the last price signal.
       * @param slot The slot to connect.
       */
      virtual boost::signals2::connection connect_last_price_slot(
        const PriceSignal::slot_type& slot) const = 0;

      /**
       * Connects a slot to the volume signal.
       * @param slot The slot to connect.
       */
      virtual boost::signals2::connection connect_volume_slot(
        const QuantitySignal::slot_type& slot) const = 0;

    protected:

      /** Constructs a technicals model. */
      TechnicalsModel() = default;

    private:
      TechnicalsModel(const TechnicalsModel&) = delete;
      TechnicalsModel& operator =(const TechnicalsModel&) = delete;
  };
}

#endif
