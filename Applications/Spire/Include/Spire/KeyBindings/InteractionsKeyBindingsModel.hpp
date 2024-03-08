#ifndef SPIRE_INTERACTIONS_KEY_BINDINGS_MODEL_HPP
#define SPIRE_INTERACTIONS_KEY_BINDINGS_MODEL_HPP
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"

namespace Spire {

  /** Stores the key bindings for interactions. */
  class InteractionsKeyBindingsModel {
    public:

      /** The number of the modifiers. */
      static constexpr auto MODIFIER_COUNT = 4;

      /** Constructs an empty model. */
      InteractionsKeyBindingsModel();

      /**
       * Constructs a model that is linked to a parent model.
       * @param parent The parent to link to.
       */
      explicit InteractionsKeyBindingsModel(
        std::shared_ptr<InteractionsKeyBindingsModel> parent);

      /** Returns whether this model has been detached from its parent. */
      bool is_detached() const;

      /** Returns the default quantity.*/
      std::shared_ptr<QuantityModel> get_default_quantity() const;

      /** Returns the quantity increment associated with a modifier.*/
      std::shared_ptr<QuantityModel>
        get_quantity_increment(Qt::KeyboardModifier modifier) const;

      /** Returns the price increment associated with a modifier.*/
      std::shared_ptr<MoneyModel>
        get_price_increment(Qt::KeyboardModifier modifier) const;

      /** Returns whether cancel on fill is enabled.*/
      std::shared_ptr<BooleanModel> is_cancel_on_fill() const;

      /** Resets this model to link to its parent. */
      void reset();

    private:
      bool m_is_detached;
      std::shared_ptr<QuantityModel> m_default_quantity;
      std::array<std::shared_ptr<QuantityModel>, MODIFIER_COUNT>
        m_quantity_increments;
      std::array<std::shared_ptr<MoneyModel>, MODIFIER_COUNT>
        m_price_increments;
      std::shared_ptr<BooleanModel> m_is_cancel_on_fill;
      Beam::SignalHandling::ConnectionGroup m_connections;

      void on_write();
  };

  /**
   * Returns the default order quantity to display to a user.
   * @param interactions The user's interactions.
   * @param security The security that the user is entering a quantity for.
   * @param position The current position in the given <i>security</i>.
   * @param side The side of the order that the user is entering a quantity for.
   * @return The default quantity to display.
   */
  Nexus::Quantity get_default_order_quantity(
    const InteractionsKeyBindingsModel& interactions,
    const Nexus::Security& security, Nexus::Quantity position,
    Nexus::Side side);
}

#endif
