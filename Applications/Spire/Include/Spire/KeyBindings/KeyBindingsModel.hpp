#ifndef SPIRE_KEY_BINDINGS_MODEL_HPP
#define SPIRE_KEY_BINDINGS_MODEL_HPP
#include <memory>
#include <unordered_map>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/KeyBindings/InteractionsKeyBindingsModel.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Stores a user's key bindings for submitting and cancelling tasks. */
  class KeyBindingsModel {
    public:

      /**
       * Constructs an empty KeyBindingsModel.
       * @param markets The database of market definitions.
       */
      explicit KeyBindingsModel(Nexus::MarketDatabase markets);

      /** Returns the list of order task arguments. */
      const std::shared_ptr<OrderTaskArgumentsListModel>&
        get_order_task_arguments() const;

      /** Returns the cancel key bindings. */
      const std::shared_ptr<CancelKeyBindingsModel>&
        get_cancel_key_bindings() const;

      /** Returns the interactions key bindings for a given region. */
      const std::shared_ptr<InteractionsKeyBindingsModel>&
        get_interactions_key_bindings(const Nexus::Region& region) const;

      /**
       * Returns the list of regions that have interactions settings available.
       */
      std::vector<Nexus::Region> make_interactions_key_bindings_regions() const;

    private:
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<OrderTaskArgumentsListModel> m_order_task_arguments;
      std::shared_ptr<CancelKeyBindingsModel> m_cancel_key_bindings;
      mutable std::unordered_map<Nexus::Region,
        std::shared_ptr<InteractionsKeyBindingsModel>> m_interactions;

      KeyBindingsModel(const KeyBindingsModel&) = delete;
      KeyBindingsModel& operator =(const KeyBindingsModel&) = delete;
  };
}

#endif
