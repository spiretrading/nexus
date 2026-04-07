#ifndef SPIRE_PORTFOLIOVIEWERPROPERTIES_HPP
#define SPIRE_PORTFOLIOVIEWERPROPERTIES_HPP
#include <unordered_set>
#include <Beam/Serialization/ShuttleUnorderedSet.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Pointers/Out.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  class UserProfile;

  /*! \class PortfolioViewerProperties
      \brief Stores the properties used by a PortfolioViewerWindow.
    */
  class PortfolioViewerProperties {
    public:

      //! Returns the default PortfolioViewerProperties.
      static PortfolioViewerProperties GetDefault();

      //! Loads the PortfolioViewerProperties from a UserProfile.
      /*!
        \param userProfile The UserProfile storing the properties.
      */
      static void Load(Beam::Out<UserProfile> userProfile);

      //! Saves a UserProfile's PortfolioViewerProperties.
      /*!
        \param userProfile The UserProfile's properties to save.
      */
      static void Save(const UserProfile& userProfile);

      //! Constructs an uninitialized PortfolioViewerProperties.
      PortfolioViewerProperties();

      //! Returns the set of selected groups.
      const std::unordered_set<Beam::DirectoryEntry>& GetSelectedGroups() const;

      //! Returns the set of selected groups.
      std::unordered_set<Beam::DirectoryEntry>& GetSelectedGroups();

      //! Returns <code>true</code> iff all groups should be selected.
      bool IsSelectingAllGroups() const;

      //! Sets whether all groups should be selected.
      void SetSelectingAllGroups(bool value);

      //! Returns the set of selected Currencies.
      const std::unordered_set<Nexus::CurrencyId>&
        GetSelectedCurrencies() const;

      //! Returns the set of selected Currencies.
      std::unordered_set<Nexus::CurrencyId>& GetSelectedCurrencies();

      //! Returns <code>true</code> iff all currencies should be selected.
      bool IsSelectingAllCurrencies() const;

      //! Sets whether all currencies should be selected.
      void SetSelectingAllCurrencies(bool value);

      //! Returns the set of selected venues.
      const std::unordered_set<Nexus::Venue>& GetSelectedVenues() const;

      //! Returns the set of selected venues.
      std::unordered_set<Nexus::Venue>& GetSelectedVenues();

      //! Returns <code>true</code> iff all venues should be selected.
      bool IsSelectingAllVenues() const;

      //! Sets whether all venues should be selected.
      void SetSelectingAllVenues(bool value);

      //! Returns the set of selected Sides.
      const std::unordered_set<Nexus::Side>& GetSelectedSides() const;

      //! Returns the set of selected Sides.
      std::unordered_set<Nexus::Side>& GetSelectedSides();

    private:
      friend struct Beam::DataShuttle;
      std::unordered_set<Beam::DirectoryEntry> m_selectedGroups;
      bool m_selectAllGroups;
      std::unordered_set<Nexus::CurrencyId> m_selectedCurrencies;
      bool m_selectAllCurrencies;
      std::unordered_set<Nexus::Venue> m_selectedVenues;
      bool m_selectAllVenues;
      std::unordered_set<Nexus::Side> m_selectedSides;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void PortfolioViewerProperties::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("selected_groups", m_selectedGroups);
    shuttle.shuttle("select_all_groups", m_selectAllGroups);
    shuttle.shuttle("selected_currencies", m_selectedCurrencies);
    shuttle.shuttle("select_all_currencies", m_selectAllCurrencies);
    shuttle.shuttle("selected_venues", m_selectedVenues);
    shuttle.shuttle("select_all_venues", m_selectAllVenues);
    shuttle.shuttle("selected_sides", m_selectedSides);
  }
}

#endif
