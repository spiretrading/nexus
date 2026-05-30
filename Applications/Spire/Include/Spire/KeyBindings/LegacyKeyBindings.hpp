#ifndef SPIRE_LEGACY_KEY_BINDINGS_HPP
#define SPIRE_LEGACY_KEY_BINDINGS_HPP
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <QKeySequence>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/ScopeMap.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Spire/HashQtTypes.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"

namespace Spire {
  class KeyBindingsModel;

  BEAM_ENUM(ModifierDefinitions,
    PLAIN,
    SHIFT,
    ALT,
    CONTROL);

  BEAM_ENUM(CancelTypeDefinitions,
    MOST_RECENT,
    MOST_RECENT_ASK,
    MOST_RECENT_BID,
    OLDEST,
    OLDEST_ASK,
    OLDEST_BID,
    ALL,
    ALL_ASKS,
    ALL_BIDS,
    CLOSEST_ASK,
    CLOSEST_BID,
    FURTHEST_ASK,
    FURTHEST_BID);

  /** Stores interactions properties in the legacy (Lollipop) format. */
  struct LegacyInteractionsProperties {
    using Modifier = ModifierDefinitions;
    static const auto MODIFIER_COUNT = 4;
    Nexus::Quantity m_default_quantity;
    std::array<Nexus::Quantity, MODIFIER_COUNT> m_quantity_increments;
    std::array<Nexus::Money, MODIFIER_COUNT> m_price_increments;
    bool m_cancel_on_fill;
  };

  /** Stores key bindings in the legacy (Lollipop) format. */
  struct LegacyKeyBindings {
    struct TaskBinding {
      std::string m_name;
      std::shared_ptr<CanvasNode> m_node;
    };
    struct CancelBinding {
      using Type = CancelTypeDefinitions;
      std::string m_description;
      Type m_type;
    };
    std::unordered_map<Nexus::Venue,
      std::unordered_map<QKeySequence, TaskBinding>> m_task_bindings;
    std::unordered_map<QKeySequence, CancelBinding> m_cancel_bindings;
    std::unordered_map<Nexus::Venue, Nexus::Quantity> m_default_quantities;
  };

  /**
   * Applies legacy key bindings and interactions onto a KeyBindingsModel,
   * resetting its order task arguments to the defaults first.
   * @param key_bindings The legacy key bindings to apply.
   * @param interactions The legacy interactions properties to apply.
   * @param model The model to apply the bindings onto.
   */
  void apply_legacy_key_bindings(const LegacyKeyBindings& key_bindings,
    const Nexus::ScopeMap<LegacyInteractionsProperties>& interactions,
    KeyBindingsModel& model);
}

namespace Beam {
  template<>
  struct Shuttle<Spire::LegacyInteractionsProperties> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::LegacyInteractionsProperties& value,
        unsigned int version) const {
      auto default_quantity = std::int64_t();
      shuttle.shuttle("default_quantity", default_quantity);
      value.m_default_quantity = default_quantity;
      auto quantity_increments = std::array<
        std::int64_t, Spire::LegacyInteractionsProperties::MODIFIER_COUNT>();
      shuttle.shuttle("quantity_increments", quantity_increments);
      for(auto i = 0;
          i < Spire::LegacyInteractionsProperties::MODIFIER_COUNT; ++i) {
        value.m_quantity_increments[i] = quantity_increments[i];
      }
      auto price_increments = std::array<
        std::int64_t, Spire::LegacyInteractionsProperties::MODIFIER_COUNT>();
      shuttle.shuttle("price_increments", price_increments);
      for(auto i = 0;
          i < Spire::LegacyInteractionsProperties::MODIFIER_COUNT; ++i) {
        value.m_price_increments[i] = Nexus::Money(
          Nexus::Quantity(price_increments[i]) / Nexus::Quantity::MULTIPLIER);
      }
      shuttle.shuttle("cancel_on_fill", value.m_cancel_on_fill);
    }
  };

  template<>
  struct Shuttle<Spire::LegacyKeyBindings::TaskBinding> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::LegacyKeyBindings::TaskBinding& value,
        unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("node", value.m_node);
    }
  };

  template<>
  struct Shuttle<Spire::LegacyKeyBindings::CancelBinding> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::LegacyKeyBindings::CancelBinding& value,
        unsigned int version) const {
      shuttle.shuttle("description", value.m_description);
      shuttle.shuttle("type", value.m_type);
    }
  };

  template<>
  struct Shuttle<Spire::LegacyKeyBindings> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::LegacyKeyBindings& value,
        unsigned int version) const {
      shuttle.shuttle("task_bindings", value.m_task_bindings);
      shuttle.shuttle("cancel_bindings", value.m_cancel_bindings);
      shuttle.shuttle("default_quantities", value.m_default_quantities);
    }
  };
}

#endif
