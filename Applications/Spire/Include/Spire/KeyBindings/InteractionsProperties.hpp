#ifndef SPIRE_INTERACTIONSPROPERTIES_HPP
#define SPIRE_INTERACTIONSPROPERTIES_HPP
#include <array>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/Spire.hpp"

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
namespace Serialization {
  template<>
  struct Shuttle<Spire::InteractionsProperties> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::InteractionsProperties& value,
        unsigned int version) {
      shuttle.Shuttle("default_quantity", value.m_defaultQuantity);
      shuttle.Shuttle("quantity_increments", value.m_quantityIncrements);
      shuttle.Shuttle("price_increments", value.m_priceIncrements);
      shuttle.Shuttle("cancel_on_fill", value.m_cancelOnFill);
    }
  };
}
}

#endif
