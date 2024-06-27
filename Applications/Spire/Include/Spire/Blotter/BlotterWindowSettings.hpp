#ifndef SPIRE_BLOTTERWINDOWSETTINGS_HPP
#define SPIRE_BLOTTERWINDOWSETTINGS_HPP
#include <QByteArray>
#include "Spire/Blotter/ActivityLogWidget.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/OpenPositionsWidget.hpp"
#include "Spire/Blotter/OrderLogWidget.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class BlotterWindowSettings
      \brief Stores the window settings for a BlotterWindow.
   */
  class BlotterWindowSettings : public LegacyUI::WindowSettings {
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
      friend struct Beam::Serialization::DataShuttle;
      std::string m_blotterName;
      QByteArray m_geometry;
      QByteArray m_splitterState;
      OpenPositionsWidget::UIState m_openPositionsWidgetState;
      OrderLogWidget::UIState m_orderLogWidgetState;
      ActivityLogWidget::UIState m_activityLogWidgetState;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void BlotterWindowSettings::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("blotter_name", m_blotterName);
    shuttle.Shuttle("geometry", m_geometry);
    shuttle.Shuttle("splitter_state", m_splitterState);
    shuttle.Shuttle("open_positions_widget_state", m_openPositionsWidgetState);
    shuttle.Shuttle("order_log_widget_state", m_orderLogWidgetState);
    shuttle.Shuttle("activity_log_widget_state", m_activityLogWidgetState);
  }
}

#endif
