#ifndef SPIRE_DASHBOARDWIDGETWINDOWSETTINGS_HPP
#define SPIRE_DASHBOARDWIDGETWINDOWSETTINGS_HPP
#include <vector>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardWidget.hpp"
#include "Spire/Dashboard/DashboardRenderer.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class DashboardWidgetWindowSettings
      \brief Stores the window settings for a DashboardWidget.
   */
  class DashboardWidgetWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a DashboardWidgetWindowSettings with default values.
      DashboardWidgetWindowSettings();

      //! Constructs a DashboardWidgetWindowSettings.
      /*!
        \param widget The DashboardWidget to represent.
      */
      DashboardWidgetWindowSettings(const DashboardWidget& widget);

      virtual ~DashboardWidgetWindowSettings();

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;
      std::vector<DashboardWidget::SortOrder> m_sortOrder;
      DashboardRendererSettings m_rendererSettings;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void DashboardWidgetWindowSettings::shuttle(S& shuttle,
      unsigned int version) {
    shuttle.shuttle("sort_order", m_sortOrder);
    shuttle.shuttle("renderer_settings", m_rendererSettings);
  }
}

#endif
