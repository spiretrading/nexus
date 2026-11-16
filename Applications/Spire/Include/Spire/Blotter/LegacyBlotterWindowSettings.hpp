#ifndef SPIRE_LEGACY_BLOTTER_WINDOW_SETTINGS_HPP
#define SPIRE_LEGACY_BLOTTER_WINDOW_SETTINGS_HPP
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"

namespace Spire {

  /**
   * Stores the legacy window settings for a BlotterWindow. Provided for
   * backward compatibility with Spire Classic.
   */
  class LegacyBlotterWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Stores a blotter table's user interface state. */
      struct TableState {

        /** The geometry of the table header. */
        QByteArray m_table_geometry;

        /** The state of the table header. */
        QByteArray m_table_state;

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };

      /** Constructs legacy blotter window settings with default values. */
      LegacyBlotterWindowSettings() = default;

      std::string GetName() const override;
      QWidget* Reopen(Beam::Ref<UserProfile> user_profile) const override;
      void Apply(Beam::Ref<UserProfile> user_profile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::DataShuttle;
      std::string m_blotter_name;
      QByteArray m_geometry;
      QByteArray m_splitter_state;
      TableState m_open_positions_widget_state;
      TableState m_order_log_widget_state;
      TableState m_activity_log_widget_state;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void LegacyBlotterWindowSettings::TableState::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("table_geometry", m_table_geometry);
    shuttle.shuttle("table_state", m_table_state);
  }

  template<Beam::IsShuttle S>
  void LegacyBlotterWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("blotter_name", m_blotter_name);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("splitter_state", m_splitter_state);
    shuttle.shuttle(
      "open_positions_widget_state", m_open_positions_widget_state);
    shuttle.shuttle("order_log_widget_state", m_order_log_widget_state);
    shuttle.shuttle("activity_log_widget_state", m_activity_log_widget_state);
  }
}

#endif
