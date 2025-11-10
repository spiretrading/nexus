#ifndef SPIRE_BLOTTERWINDOWSETTINGS_HPP
#define SPIRE_BLOTTERWINDOWSETTINGS_HPP
#include <QByteArray>
#include "Spire/Blotter/ActivityLogWidget.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/OpenPositionsWidget.hpp"
#include "Spire/Blotter/OrderLogWidget.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class BlotterWindowSettings
      \brief Stores the window settings for a BlotterWindow.
   */
  class BlotterWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a BlotterWindowSettings with default values.
      BlotterWindowSettings();

      //! Constructs a BlotterWindowSettings.
      /*!
        \param window The BlotterWindow to represent.
      */
      BlotterWindowSettings(const BlotterWindow& window);

      virtual ~BlotterWindowSettings();

      //! Returns the blotter's name.
      const std::string& GetBlotterName() const;

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;
      std::string m_blotterName;
      QByteArray m_geometry;
      QByteArray m_splitterState;
      OpenPositionsWidget::UIState m_openPositionsWidgetState;
      OrderLogWidget::UIState m_orderLogWidgetState;
      ActivityLogWidget::UIState m_activityLogWidgetState;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BlotterWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("blotter_name", m_blotterName);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("splitter_state", m_splitterState);
    shuttle.shuttle("open_positions_widget_state", m_openPositionsWidgetState);
    shuttle.shuttle("order_log_widget_state", m_orderLogWidgetState);
    shuttle.shuttle("activity_log_widget_state", m_activityLogWidgetState);
  }
}

#endif
