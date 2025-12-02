#ifndef SPIRE_CHARTPLOTVIEW_HPP
#define SPIRE_CHARTPLOTVIEW_HPP
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include <QWidget>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/Charting/ChartInteractions.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"

namespace Spire {

  /*! \class ChartPlotView
      \brief Displays ChartPlots.
   */
  class ChartPlotView : public QWidget, public LegacyUI::PersistentWindow {
    public:

      /*! \struct Properties
          \brief Stores the properties used to display a chart.
       */
      struct Properties {

        //! Returns the default properties.
        static Properties GetDefault();

        //! The background color.
        QColor m_backgroundColor;

        //! The color of a grid line.
        QColor m_gridLineColor;

        //! The color of the grid labels.
        QColor m_gridLabelColor;

        //! The font used for the grid labels.
        QFont m_gridLabelFont;

        //! The color used for an uptick.
        QColor m_uptickColor;

        //! The color used for a downtick.
        QColor m_downtickColor;

        //! The color used to outline bars.
        QColor m_outlineColor;
      };

      /*! \struct AxisParameters
          \brief Stores the parameters for displaying an axis.
       */
      struct AxisParameters {

        //! The data-type represented by the axis.
        std::shared_ptr<NativeType> m_type;

        //! The minimum value to display.
        ChartValue m_min;

        //! The maximum value to display.
        ChartValue m_max;

        //! The size of a single increment.
        ChartValue m_increment;

        //! Constructs an uninitialized AxisParameters instance.
        AxisParameters();

        //! Constructs an AxisParameters instance.
        /*!
          \param type The data-type represented by the axis.
          \param min The minimum value to display.
          \param max The maximum value to display.
          \param increment The size of a single increment.
        */
        AxisParameters(std::shared_ptr<NativeType> type, ChartValue min,
          ChartValue max, ChartValue increment);
      };

      //! Signals a change to one of the AxisParameters.
      /*!
        \param parameters The updated AxisParameters.
      */
      typedef boost::signals2::signal<void (const AxisParameters& parameters)>
        AxisParametersChangedSignal;

      //! Signals that the view is about to be panned.
      typedef boost::signals2::signal<void ()> BeginPanSignal;

      //! Signals that the view is no longer being panned.
      typedef boost::signals2::signal<void ()> EndPanSignal;

      //! Constructs a ChartPlotView, must be initialized after construction.
      /*!
        \param parent The parent widget.
      */
      ChartPlotView(QWidget* parent = nullptr);

      virtual ~ChartPlotView() = default;

      //! Initializes this widget.
      /*!
        \param userProfile The user's profile.
        \param properties The properties used to display the chart.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        const Properties& properties);

      //! Returns the interaction mode.
      ChartInteractionMode GetInteractionMode() const;

      //! Sets the interaction mode.
      void SetInteractionMode(ChartInteractionMode interactionMode);

      //! Returns the display properties.
      const Properties& GetProperties() const;

      //! Sets the display properties.
      void SetProperties(const Properties& properties);

      //! Returns the x-axis parameters.
      const AxisParameters& GetXAxisParameters() const;

      //! Sets the x-axis parameters.
      /*!
        \param parameters The AxisParameters to use for the x-axis.
      */
      void SetXAxisParameters(const AxisParameters& parameters);

      //! Returns the y-axis parameters.
      const AxisParameters& GetYAxisParameters() const;

      //! Sets the y-axis parameters.
      /*!
        \param parameters The AxisParameters to use for the y-axis.
      */
      void SetYAxisParameters(const AxisParameters& parameters);

      //! Plots a point on this chart.
      /*!
        \param plot The point to plot.
      */
      void Plot(const std::shared_ptr<ChartPlot>& plot);

      //! Clears all plots.
      void Clear();

      //! Connects a slot to the XAxisParametersChangedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectXAxisParametersChangedSignal(
        const AxisParametersChangedSignal::slot_function_type& slot) const;

      //! Connects a slot to the YAxisParametersChangedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectYAxisParametersChangedSignal(
        const AxisParametersChangedSignal::slot_function_type& slot) const;

      //! Connects a slot to the BeginPanSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectBeginPanSignal(
        const BeginPanSignal::slot_function_type& slot) const;

      //! Connects a slot to the EndPanSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectEndPanSignal(
        const EndPanSignal::slot_function_type& slot) const;

      virtual std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const;

    protected:
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      virtual void paintEvent(QPaintEvent* event);
      virtual void wheelEvent(QWheelEvent* event);

    private:
      UserProfile* m_userProfile;
      ChartInteractionMode m_interactionMode;
      bool m_isDragging;
      QPoint m_previousMousePosition;
      Properties m_properties;
      AxisParameters m_xAxisParameters;
      AxisParameters m_yAxisParameters;
      std::vector<std::shared_ptr<ChartPlot>> m_plots;
      Beam::ConnectionGroup m_plotConnections;
      mutable BeginPanSignal m_beginPanSignal;
      mutable EndPanSignal m_endPanSignal;
      mutable AxisParametersChangedSignal m_xAxisParametersChangedSignal;
      mutable AxisParametersChangedSignal m_yAxisParametersChangedSignal;

      void PaintGrids();
      void PaintChartPlots();
      void PaintCrossHairs();
      void PaintCandlestickChartPlot(const CandlestickChartPlot& plot);
      void PaintHorizontalCursor(const QPoint& position,
        const ChartValue& value);
      void PaintVerticalCursor(const QPoint& position, const ChartValue& value);
      void Zoom(ChartValue x, ChartValue y, int percentage);
      void Drag(ChartValue xIncrement, ChartValue yIncrement);
      std::tuple<ChartValue, ChartValue> ComputeChartPoint(const QPoint& point);
      QPoint ComputeScreenPoint(ChartValue x, ChartValue y);
      QString LoadLabel(ChartValue value, const NativeType& type) const;
      int GetChartWidth() const;
      int GetChartHeight() const;
      void OnPlotUpdate();
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::ChartPlotView::Properties> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::ChartPlotView::Properties& value,
        unsigned int version) const {
      shuttle.shuttle("background_color", value.m_backgroundColor);
      shuttle.shuttle("grid_line_color", value.m_gridLineColor);
      shuttle.shuttle("grid_label_color", value.m_gridLabelColor);
      shuttle.shuttle("grid_label_font", value.m_gridLabelFont);
      shuttle.shuttle("uptick_color", value.m_uptickColor);
      shuttle.shuttle("downtick_color", value.m_downtickColor);
      shuttle.shuttle("outline_color", value.m_outlineColor);
    }
  };

  template<>
  struct Shuttle<Spire::ChartPlotView::AxisParameters> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::ChartPlotView::AxisParameters& value,
        unsigned int version) const {
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("min", value.m_min);
      shuttle.shuttle("max", value.m_max);
      shuttle.shuttle("increment", value.m_increment);
    }
  };
}

#endif
