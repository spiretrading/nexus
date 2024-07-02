#ifndef SPIRE_CHARTPLOTVIEWWINDOWSETTINGS_HPP
#define SPIRE_CHARTPLOTVIEWWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <QByteArray>
#include "Spire/Charting/ChartPlotView.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class ChartPlotViewWindowSettings
      \brief Stores the window settings for a ChartPlotView.
   */
  class ChartPlotViewWindowSettings : public LegacyUI::WindowSettings {
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
      friend struct Beam::Serialization::DataShuttle;
      ChartPlotView::Properties m_properties;
      ChartPlotView::AxisParameters m_xAxisParameters;
      ChartPlotView::AxisParameters m_yAxisParameters;
      QByteArray m_geometry;

      ChartPlotViewWindowSettings();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ChartPlotViewWindowSettings::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("properties", m_properties);
    shuttle.Shuttle("x_axis_parameters", m_xAxisParameters);
    shuttle.Shuttle("y_axis_parameters", m_yAxisParameters);
    shuttle.Shuttle("geometry", m_geometry);
  }
}

#endif
