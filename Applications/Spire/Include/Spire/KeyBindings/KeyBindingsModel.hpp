#ifndef SPIRE_KEY_BINDINGS_MODEL_HPP
#define SPIRE_KEY_BINDINGS_MODEL_HPP
#include "Spire/KeyBindings/OrderTask.hpp"
#include "Spire/Ui/ComboBox.hpp"

namespace Spire {

  /** Defines the interface representing key bindings */
  class KeyBindingsModel {
    public:
      virtual ~KeyBindingsModel() = default;

      /** Returns the key bindings of the order tasks. */
      virtual std::shared_ptr<ListModel<OrderTask>> get_order_tasks() = 0;

      /** Returns the default key bindings of the order tasks. */
      virtual const std::shared_ptr<ListModel<OrderTask>>
        get_default_order_tasks() const = 0;

      /** Returns the region query model. */
      virtual std::shared_ptr<ComboBox::QueryModel>
        get_region_query_model() = 0;

      /** Returns the destination database to use. */
      virtual const Nexus::DestinationDatabase& get_destinations() const = 0;

      /** Returns the market database to use. */
      virtual const Nexus::MarketDatabase& get_markets() const = 0;

    protected:

      /** Constructs a KeyBindingsModel. */
      KeyBindingsModel() = default;

    private:
      KeyBindingsModel(const KeyBindingsModel&) = delete;
      KeyBindingsModel& operator =(const KeyBindingsModel&) = delete;
  };
}

#endif
