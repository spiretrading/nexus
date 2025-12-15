#ifndef SPIRE_LEGACY_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#define SPIRE_LEGACY_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#include <array>
#include <Beam/Serialization/ShuttleArray.hpp>
#include <QByteArray>
#include <QColor>
#include <QFont>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"

namespace Spire {

  /** 
   * Stores the legacy window settings for a TimeAndSalesWindow. Provided for
   * backward compatibility with Spire Classic.
   */
  class LegacyTimeAndSalesWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Stores the time and sales properties used by Spire Classic. */
      struct Properties {

        /** The number of enumerated price ranges. */
        static const auto PRICE_RANGE_COUNT = 6;

        /** The number of enumerated columns. */
        static const auto COLUMN_COUNT = 5;

        /** The foreground colors used for the price ranges. */
        std::array<QColor, PRICE_RANGE_COUNT> m_price_range_foreground_color;

        /** The background colors used for the price ranges. */
        std::array<QColor, PRICE_RANGE_COUNT> m_price_range_background_color;

        /** The visible columns. */
        std::array<bool, COLUMN_COUNT> m_visible_columns;

        /** Whether the grid lines are visible. */
        bool m_show_grid_lines;

        /** Whether the vertical scrollbar is visible. */
        bool m_vertical_scroll_bar_visible;

        /** Whether the horizontal scrollbar is visible. */
        bool m_horizontal_scroll_bar_visible;

        /** he display font. */
        QFont m_font;
      };

      /** Constructs a LegacyTimeAndSalesWindowSettings with default values. */
      LegacyTimeAndSalesWindowSettings() = default;

      std::string GetName() const override;

      QWidget* Reopen(Beam::Ref<UserProfile> user_profile) const override;

      void Apply(Beam::Ref<UserProfile> user_profile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::DataShuttle;
      Properties m_properties;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_security_view_stack;
      std::string m_identifier;
      std::string m_link_identifier;
      QByteArray m_geometry;
      QByteArray m_splitter_state;
      QByteArray m_view_header_state;
      QByteArray m_snapshot_header_state;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void LegacyTimeAndSalesWindowSettings::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("security", m_security);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("security_view_stack", m_security_view_stack);
    shuttle.shuttle("identifier", m_identifier);
    shuttle.shuttle("link_identifier", m_link_identifier);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("splitter_state", m_splitter_state);
    shuttle.shuttle("view_header_state", m_view_header_state);
    shuttle.shuttle("snapshot_header_state", m_snapshot_header_state);
  }
}

namespace Beam {
  template<>
  struct Shuttle<Spire::LegacyTimeAndSalesWindowSettings::Properties> {
    template<IsShuttle S>
    void operator ()(S& shuttle,
        Spire::LegacyTimeAndSalesWindowSettings::Properties& value,
        unsigned int version) {
      shuttle.shuttle("price_range_foreground_color",
        value.m_price_range_foreground_color);
      shuttle.shuttle("price_range_background_color",
        value.m_price_range_background_color);
      shuttle.shuttle("visible_columns", value.m_visible_columns);
      shuttle.shuttle("show_grid_lines", value.m_show_grid_lines);
      shuttle.shuttle(
        "vertical_scroll_bar_visible", value.m_vertical_scroll_bar_visible);
      shuttle.shuttle("horizontal_scroll_bar_visible",
        value.m_horizontal_scroll_bar_visible);
      shuttle.shuttle("font", value.m_font);
    }
  };
}

#endif
