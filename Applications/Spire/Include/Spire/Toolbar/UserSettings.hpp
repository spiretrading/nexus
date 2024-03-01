#ifndef SPIRE_ENVIRONMENT_SETTINGS_HPP
#define SPIRE_ENVIRONMENT_SETTINGS_HPP
#include <filesystem>
#include <Beam/Collections/EnumSet.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/RegionMap.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/LegacyKeyBindings/InteractionsProperties.hpp"
#include "Spire/LegacyKeyBindings/KeyBindings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

namespace Spire {
namespace Details {

  /** Enumerates the types of settings a user can store. */
  BEAM_ENUM(Category,

    /** The book view. */
    BOOK_VIEW,

    /** The order imbalance indicator. */
    ORDER_IMBALANCE_INDICATOR,

    /** The interactions. */
    INTERACTIONS,

    /** The key bindings. */
    KEY_BINDINGS,

    /** The portfolio. */
    PORTFOLIO,

    /** The time and sales. */
    TIME_AND_SALES,

    /** The watchlist. */
    WATCHLIST,

    /** The layout. */
    LAYOUT
  );
}

  /** Stores a variety of settings that can be exported by a user. */
  struct UserSettings {
    using Category = Details::Category;

    /** A Category set represented by a bitset. */
    using Categories = Beam::EnumSet<Category>;

    /** Defines a value model over the Settings. */
    using CategoriesModel = ValueModel<Categories>;

    /** Defines a local value model over the Categories. */
    using LocalCategoriesModel = LocalValueModel<Categories>;

    /** The book view properties. */
    boost::optional<BookViewProperties> m_book_view_properties;

    /** The dashboards. */
    boost::optional<SavedDashboards> m_dashboards;

    /** The OrderImbalance indicator properties. */
    boost::optional<OrderImbalanceIndicatorProperties>
      m_order_imbalance_indicator_properties;

    /** The interactions properties. */
    boost::optional<Nexus::RegionMap<InteractionsProperties>>
      m_interactions_properties;

    /** The key bindings. */
    boost::optional<KeyBindings> m_key_bindings;

    /** The portfolio properties. */
    boost::optional<PortfolioViewerProperties> m_portfolio_properties;

    /** The time and sales properties. */
    boost::optional<TimeAndSalesProperties> m_time_and_sales_properties;

    /** The window layouts. */
    boost::optional<std::vector<std::shared_ptr<LegacyUI::WindowSettings>>>
      m_layouts;
  };

  /** Returns the text representation of a Category. */ 
  const QString& to_text(UserSettings::Category category);

  /**
   * Exports the user's settings.
   * @param settings The UserSettings to save.
   * @param path The file path to save the settings to.
   */
  void export_settings(
    const UserSettings& settings, const std::filesystem::path& path);

  /**
   * Imports the user's settings.
   * @param categories The categories to import.
   * @param path The file path to import.
   * @param user_profile The user's profile.
   */
  void import_settings(UserSettings::Categories categories,
    const std::filesystem::path& path, Beam::Out<UserProfile> userProfile);
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Spire::UserSettings> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::UserSettings& value,
        unsigned int version) {
      shuttle.Shuttle("book_view_properties", value.m_book_view_properties);
      shuttle.Shuttle("dashboards", value.m_dashboards);
      shuttle.Shuttle("order_imbalance_indicator_properties",
        value.m_order_imbalance_indicator_properties);
      shuttle.Shuttle("interactions_properties",
        value.m_interactions_properties);
      shuttle.Shuttle("key_bindings", value.m_key_bindings);
      shuttle.Shuttle("portfolio_viewer_properties",
        value.m_portfolio_properties);
      shuttle.Shuttle("time_and_sales_properties",
        value.m_time_and_sales_properties);
      shuttle.Shuttle("window_layouts", value.m_layouts);
    }
  };
}

#endif
