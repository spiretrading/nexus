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

      /** Returns the global interactions key bindings. */
      const std::shared_ptr<InteractionsKeyBindingsModel>&
        get_interactions_key_bindings() const;

      /**
       * Returns a country's interactions key bindings.
       * @param country The country to lookup the interactions for.
       */
      const std::shared_ptr<InteractionsKeyBindingsModel>&
        get_interactions_key_bindings(Nexus::CountryCode country) const;

      /**
       * Returns a market's interactions key bindings.
       * @param market The market to lookup the interactions for.
       */
      const std::shared_ptr<InteractionsKeyBindingsModel>&
        get_interactions_key_bindings(Nexus::MarketCode market) const;

      /**
       * Returns a security's interactions key bindings.
       * @param security The security to lookup the interactions for.
       */
      const std::shared_ptr<InteractionsKeyBindingsModel>&
        get_interactions_key_bindings(const Nexus::Security& security) const;

    private:
      Nexus::MarketDatabase m_markets;
      std::shared_ptr<OrderTaskArgumentsListModel> m_order_task_arguments;
      std::shared_ptr<CancelKeyBindingsModel> m_cancel_key_bindings;
      std::shared_ptr<InteractionsKeyBindingsModel> m_global_interactions;
      mutable std::unordered_map<Nexus::CountryCode,
        std::shared_ptr<InteractionsKeyBindingsModel>> m_country_interactions;
      mutable std::unordered_map<Nexus::MarketCode,
        std::shared_ptr<InteractionsKeyBindingsModel>> m_market_interactions;
      mutable std::unordered_map<Nexus::Security,
        std::shared_ptr<InteractionsKeyBindingsModel>> m_security_interactions;

      KeyBindingsModel(const KeyBindingsModel&) = delete;
      KeyBindingsModel& operator =(const KeyBindingsModel&) = delete;
  };
}

#endif
