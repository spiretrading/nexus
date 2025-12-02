#ifndef SPIRE_CHARTPLOTVIEWWINDOWSETTINGS_HPP
#define SPIRE_CHARTPLOTVIEWWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <QByteArray>
#include "Spire/Charting/ChartPlotView.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class ChartPlotViewWindowSettings
      \brief Stores the window settings for a ChartPlotView.
   */
  class ChartPlotViewWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a ChartPlotViewWindowSettings.
      /*!
        \param widget The widget to represent.
      */
      ChartPlotViewWindowSettings(const ChartPlotView& widget);

      virtual ~ChartPlotViewWindowSettings();

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;
      ChartPlotView::Properties m_properties;
      ChartPlotView::AxisParameters m_xAxisParameters;
      ChartPlotView::AxisParameters m_yAxisParameters;
      QByteArray m_geometry;

      ChartPlotViewWindowSettings();
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void ChartPlotViewWindowSettings::shuttle(S& shuttle,
      unsigned int version) {
    shuttle.shuttle("properties", m_properties);
    shuttle.shuttle("x_axis_parameters", m_xAxisParameters);
    shuttle.shuttle("y_axis_parameters", m_yAxisParameters);
    shuttle.shuttle("geometry", m_geometry);
  }
}

#endif
