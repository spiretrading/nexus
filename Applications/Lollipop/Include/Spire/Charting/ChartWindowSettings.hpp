#ifndef SPIRE_CHARTWINDOWSETTINGS_HPP
#define SPIRE_CHARTWINDOWSETTINGS_HPP
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <QByteArray>
#include "Nexus/Definitions/Ticker.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartInteractions.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/TickerViewStack.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class ChartWindowSettings
      \brief Stores the window settings for a ChartWindow.
   */
  class ChartWindowSettings : public UI::WindowSettings {
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
      friend struct Beam::DataShuttle;
      ChartInteractionMode m_interactionMode;
      bool m_isAutoScaleEnabled;
      bool m_isLockGridEnabled;
      Nexus::Ticker m_ticker;
      std::string m_name;
      UI::TickerViewStack m_tickerViewStack;
      std::string m_identifier;
      std::string m_linkIdentifier;
      QByteArray m_geometry;
      std::unique_ptr<UI::WindowSettings> m_chartPlotViewWindowSettings;
      std::unique_ptr<UI::WindowSettings> m_chartIntervalComboBoxWindowSettings;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void ChartWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("interaction_mode", m_interactionMode);
    shuttle.shuttle("is_auto_scale_enabled", m_isAutoScaleEnabled);
    shuttle.shuttle("is_lock_grid_enabled", m_isLockGridEnabled);
    shuttle.shuttle("ticker", m_ticker);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("ticker_view_stack", m_tickerViewStack);
    shuttle.shuttle("identifier", m_identifier);
    shuttle.shuttle("link_identifier", m_linkIdentifier);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("chart_plot_view_window_settings",
      m_chartPlotViewWindowSettings);
    shuttle.shuttle("chart_interval_combox_box_window_settings",
      m_chartIntervalComboBoxWindowSettings);
  }
}

#endif
