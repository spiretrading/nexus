#ifndef SPIRE_DASHBOARDWINDOWSETTINGS_HPP
#define SPIRE_DASHBOARDWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUniquePtr.hpp>
#include <QByteArray>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardModelSchema.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"

namespace Spire {

  /*! \class DashboardWindowSettings
      \brief Stores the window settings for a DashboardWindow.
   */
  class DashboardWindowSettings : public LegacyUI::WindowSettings {
    public:

      //! Constructs a DashboardWindowSettings with default values.
      DashboardWindowSettings();

      //! Constructs a DashboardWindowSettings from a DashboardWindow.
      /*!
        \param window The DashboardWindow to store.
      */
      DashboardWindowSettings(const DashboardWindow& window);

      virtual ~DashboardWindowSettings();

      virtual std::string GetName() const;
      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;
      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;
      std::string m_name;
      DashboardModelSchema m_schema;
      std::unique_ptr<LegacyUI::WindowSettings> m_dashboardWidgetSettings;
      QByteArray m_geometry;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DashboardWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("schema", m_schema);
    shuttle.shuttle("dashboard_widget_settings", m_dashboardWidgetSettings);
    shuttle.shuttle("geometry", m_geometry);
  }
}

#endif
