#ifndef SPIRE_ENVIRONMENTSETTINGS_HPP
#define SPIRE_ENVIRONMENTSETTINGS_HPP
#include <filesystem>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/RegionMap.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {
namespace Details {
  BEAM_ENUM(EnvironmentSettingsType,

    //! The book view properties.
    BOOK_VIEW,

    //! The dashboards.
    DASHBOARDS,

    //! The OrderImbalance indicator properties.
    ORDER_IMBALANCE_INDICATOR,

    //! The interactions properties.
    INTERACTIONS,

    //! The key bindings.
    KEY_BINDINGS,

    //! The portfolio viewer properties.
    PORTFOLIO_VIEWER,

    //! The time and sales properties.
    TIME_AND_SALES,

    //! The window layouts.
    WINDOW_LAYOUTS);
}

  /*! \struct EnvironmentSettings
      \brief Stores a variety of settings that can be exported by a user.
   */
  struct EnvironmentSettings {

    //! Enumerates the types of settings stored.
    using Type = Details::EnvironmentSettingsType;

    //! Stores a set of setting types.
    using TypeSet = Beam::EnumSet<Type>;

    //! The book view properties.
    boost::optional<BookViewProperties> m_bookViewProperties;

    //! The dashboards.
    boost::optional<SavedDashboards> m_dashboards;

    //! The OrderImbalance indicator properties.
    boost::optional<OrderImbalanceIndicatorProperties>
      m_orderImbalanceIndicatorProperties;

    //! The interactions properties.
    boost::optional<Nexus::RegionMap<InteractionsProperties>>
      m_interactionsProperties;

    //! The key bindings.
    boost::optional<KeyBindings> m_keyBindings;

    //! The portfolio viewer properties.
    boost::optional<PortfolioViewerProperties> m_portfolioViewerProperties;

    //! The time and sales properties.
    boost::optional<TimeAndSalesProperties> m_timeAndSalesProperties;

    //! The window layouts.
    boost::optional<std::vector<std::shared_ptr<WindowSettings>>>
      m_windowLayouts;
  };

  //! Exports a user's EnvironmentSettings.
  /*!
    \param environmentSettings The EnvironmentSettings to save.
    \param environmentPath The file path to save the settings to.
    \return <code>true</code> iff the settings were exported successfully.
  */
  bool Export(const EnvironmentSettings& environmentSettings,
    const std::filesystem::path& environmentPath);

  //! Imports a user's EnvironmentSettings.
  /*!
    \param environmentPath The file path to import.
    \param settings The settings to import.
    \param apply Whether to apply the settings to all open windows.
    \param userProfile The user's profile.
    \return <code>true</code> iff the settings were imported successfully.
  */
  bool Import(const std::filesystem::path& environmentPath,
    EnvironmentSettings::TypeSet settings, bool apply,
    Beam::Out<UserProfile> userProfile);
}
}

namespace Beam {
  template<>
  struct Shuttle<Spire::UI::EnvironmentSettings> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::UI::EnvironmentSettings& value,
        unsigned int version) const {
      shuttle.shuttle("book_view_properties", value.m_bookViewProperties);
      shuttle.shuttle("dashboards", value.m_dashboards);
      shuttle.shuttle("order_imbalance_indicator_properties",
        value.m_orderImbalanceIndicatorProperties);
      shuttle.shuttle("interactions_properties",
        value.m_interactionsProperties);
      shuttle.shuttle("key_bindings", value.m_keyBindings);
      shuttle.shuttle("portfolio_viewer_properties",
        value.m_portfolioViewerProperties);
      shuttle.shuttle("time_and_sales_properties",
        value.m_timeAndSalesProperties);
      shuttle.shuttle("window_layouts", value.m_windowLayouts);
    }
  };
}

#endif
