#ifndef SPIRE_INTERACTIONSPROPERTIES_HPP
#define SPIRE_INTERACTIONSPROPERTIES_HPP
#include <array>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/RegionMap.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /*! \struct InteractionsProperties
      \brief Stores user properties for interacting with CanvasViews.
   */
  struct InteractionsProperties {

    //! Returns the default InteractionsProperties.
    static InteractionsProperties GetDefaultProperties();

    //! Returns the default InteractionsProperties.
    /*!
      \param userProfile The user's profile.
    */
    static Nexus::RegionMap<InteractionsProperties> GetDefaultPropertiesMap(
      const UserProfile& userProfile);

    //! Loads the InteractionsProperties from a UserProfile.
    /*!
      \param userProfile The UserProfile to load the properties from.
    */
    static void Load(Beam::Out<UserProfile> userProfile);

    //! Saves a UserProfile's InteractionsProperties.
    /*!
      \param userProfile The UserProfile's properties to save.
    */
    static void Save(const UserProfile& userProfile);

    //! The default quantity to display.
    Nexus::Quantity m_defaultQuantity;

    //! The amount to increment the quantity field by.
    std::array<Nexus::Quantity, KeyModifiers::COUNT> m_quantityIncrements;

    //! The amount to increment the price field by.
    std::array<Nexus::Money, KeyModifiers::COUNT> m_priceIncrements;

    //! Whether to cancel all Orders on a given Security's Side when one of them
    //! is filled.
    bool m_cancelOnFill;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::InteractionsProperties> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::InteractionsProperties& value,
        unsigned int version) const {
      if(Beam::IsReceiver<S>) {
        std::int64_t defaultQuantity;
        shuttle.shuttle("default_quantity", defaultQuantity);
        value.m_defaultQuantity = defaultQuantity;
        std::array<std::int64_t, Spire::KeyModifiers::COUNT> quantityIncrements;
        shuttle.shuttle("quantity_increments", quantityIncrements);
        for(auto i = 0; i < Spire::KeyModifiers::COUNT; ++i) {
          value.m_quantityIncrements[i] = quantityIncrements[i];
        }
        std::array<std::int64_t, Spire::KeyModifiers::COUNT> priceIncrements;
        shuttle.shuttle("price_increments", priceIncrements);
        for(auto i = 0; i < Spire::KeyModifiers::COUNT; ++i) {
          value.m_priceIncrements[i] = Nexus::Money{
            Nexus::Quantity{priceIncrements[i]} / Nexus::Quantity::MULTIPLIER};
        }
      } else {
        std::int64_t defaultQuantity = static_cast<std::int64_t>(
          value.m_defaultQuantity);
        shuttle.shuttle("default_quantity", defaultQuantity);
        std::array<std::int64_t, Spire::KeyModifiers::COUNT> quantityIncrements;
        for(auto i = 0; i < Spire::KeyModifiers::COUNT; ++i) {
          quantityIncrements[i] = static_cast<std::int64_t>(
            value.m_quantityIncrements[i]);
        }
        shuttle.shuttle("quantity_increments", quantityIncrements);
        std::array<std::int64_t, Spire::KeyModifiers::COUNT> priceIncrements;
        for(auto i = 0; i < Spire::KeyModifiers::COUNT; ++i) {
          priceIncrements[i] = static_cast<std::int64_t>(
            static_cast<Nexus::Quantity>(value.m_priceIncrements[i]) *
            Nexus::Quantity::MULTIPLIER);
        }
        shuttle.shuttle("price_increments", priceIncrements);
      }
      shuttle.shuttle("cancel_on_fill", value.m_cancelOnFill);
    }
  };
}

#endif
