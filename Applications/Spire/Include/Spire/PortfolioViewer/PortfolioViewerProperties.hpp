#ifndef SPIRE_PORTFOLIOVIEWERPROPERTIES_HPP
#define SPIRE_PORTFOLIOVIEWERPROPERTIES_HPP
#include <unordered_set>
#include <Beam/Serialization/ShuttleUnorderedSet.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Pointers/Out.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

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
      const std::unordered_set<Beam::ServiceLocator::DirectoryEntry>&
        GetSelectedGroups() const;

      //! Returns the set of selected groups.
      std::unordered_set<Beam::ServiceLocator::DirectoryEntry>&
        GetSelectedGroups();

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

      //! Returns the set of selected markets.
      const std::unordered_set<Nexus::MarketCode>& GetSelectedMarkets() const;

      //! Returns the set of selected markets.
      std::unordered_set<Nexus::MarketCode>& GetSelectedMarkets();

      //! Returns <code>true</code> iff all markets should be selected.
      bool IsSelectingAllMarkets() const;

      //! Sets whether all markets should be selected.
      void SetSelectingAllMarkets(bool value);

      //! Returns the set of selected Sides.
      const std::unordered_set<Nexus::Side>& GetSelectedSides() const;

      //! Returns the set of selected Sides.
      std::unordered_set<Nexus::Side>& GetSelectedSides();

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::unordered_set<Beam::ServiceLocator::DirectoryEntry> m_selectedGroups;
      bool m_selectAllGroups;
      std::unordered_set<Nexus::CurrencyId> m_selectedCurrencies;
      bool m_selectAllCurrencies;
      std::unordered_set<Nexus::MarketCode> m_selectedMarkets;
      bool m_selectAllMarkets;
      std::unordered_set<Nexus::Side> m_selectedSides;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void PortfolioViewerProperties::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("selected_groups", m_selectedGroups);
    shuttle.Shuttle("select_all_groups", m_selectAllGroups);
    shuttle.Shuttle("selected_currencies", m_selectedCurrencies);
    shuttle.Shuttle("select_all_currencies", m_selectAllCurrencies);
    shuttle.Shuttle("selected_markets", m_selectedMarkets);
    shuttle.Shuttle("select_all_markets", m_selectAllMarkets);
    shuttle.Shuttle("selected_sides", m_selectedSides);
  }
}

#endif
