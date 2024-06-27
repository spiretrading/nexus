#ifndef SPIRE_CHARTWINDOWSETTINGS_HPP
#define SPIRE_CHARTWINDOWSETTINGS_HPP
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <QByteArray>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartInteractions.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class ChartWindowSettings
      \brief Stores the window settings for a ChartWindow.
   */
  class ChartWindowSettings : public LegacyUI::WindowSettings {
    public:

      //! Constructs a ChartWindowSettings with default values.
      ChartWindowSettings();

      //! Constructs a ChartWindowSettings.
      /*!
        \param window The ChartWindow to represent.
        \param userProfile The user's profile.
      */
      ChartWindowSettings(const ChartWindow& window,
        Beam::Ref<UserProfile> userProfile);

      virtual ~ChartWindowSettings();

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      ChartInteractionMode m_interactionMode;
      bool m_isAutoScaleEnabled;
      bool m_isLockGridEnabled;
      Nexus::Security m_security;
      std::string m_name;
      LegacyUI::SecurityViewStack m_securityViewStack;
      std::string m_identifier;
      std::string m_linkIdentifier;
      QByteArray m_geometry;
      std::unique_ptr<LegacyUI::WindowSettings> m_chartPlotViewWindowSettings;
      std::unique_ptr<LegacyUI::WindowSettings>
        m_chartIntervalComboBoxWindowSettings;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ChartWindowSettings::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("interaction_mode", m_interactionMode);
    shuttle.Shuttle("is_auto_scale_enabled", m_isAutoScaleEnabled);
    shuttle.Shuttle("is_lock_grid_enabled", m_isLockGridEnabled);
    shuttle.Shuttle("security", m_security);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("security_view_stack", m_securityViewStack);
    shuttle.Shuttle("identifier", m_identifier);
    shuttle.Shuttle("link_identifier", m_linkIdentifier);
    shuttle.Shuttle("geometry", m_geometry);
    shuttle.Shuttle("chart_plot_view_window_settings",
      m_chartPlotViewWindowSettings);
    shuttle.Shuttle("chart_interval_combox_box_window_settings",
      m_chartIntervalComboBoxWindowSettings);
  }
}

#endif
