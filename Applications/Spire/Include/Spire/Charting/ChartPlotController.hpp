#ifndef SPIRE_CHARTPLOTCONTROLLER_HPP
#define SPIRE_CHARTPLOTCONTROLLER_HPP
#include <optional>
#include <unordered_map>
#include <vector>
#include <Beam/Collections/SortedVector.hpp>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <QTimer>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartPlotView.hpp"

namespace Spire {

  /*! \class ChartPlotController
      \brief Serves as controller between a single ChartPlotView and the many
             ChartPlotSeries that populate it.
   */
  class ChartPlotController : private boost::noncopyable {
    public:

      //! Constructs a ChartPlotController.
      ChartPlotController();

      //! Returns <code>true</code> iff auto-scale is enabled.
      bool IsAutoScaleEnabled() const;

      //! Sets whether auto-scale is enabled.
      void SetAutoScale(bool autoScale);

      //! Returns <code>true</code> iff the grid is locked.
      bool IsLockGridEnabled() const;

      //! Sets whether the grid is locked.
      void SetLockGrid(bool lockGrid);

      //! Sets the view to populate with ChartPlots.
      /*!
        \param view The view to populate with ChartPlots.
      */
      void SetView(Beam::Ref<ChartPlotView> view);

      //! Adds a ChartPlotSeries to populate the ChartPlotView with.
      /*!
        \param series The ChartPlotSeries to add.
      */
      void Add(const std::shared_ptr<ChartPlotSeries>& series);

      //! Clears all ChartPlotSeries.
      void Clear();

    private:
      struct ChartPlotComparator {
        bool operator ()(const std::shared_ptr<ChartPlot>& a,
          const std::shared_ptr<ChartPlot>& b) const;
      };
      struct RangeEntry {
        ChartValue m_min;
        ChartValue m_max;

        RangeEntry(ChartValue min, ChartValue max);
      };
      ChartPlotView* m_view;
      bool m_isAutoScaleEnabled;
      bool m_isLockGridEnabled;
      std::vector<std::shared_ptr<ChartPlotSeries>> m_series;
      std::optional<ChartValue> m_lastValue;
      std::uint64_t m_lastValueIndex;
      Beam::SynchronizedList<std::vector<std::tuple<std::uint64_t, ChartValue>>>
        m_lastValuesLoaded;
      Beam::SortedVector<std::shared_ptr<ChartPlot>, ChartPlotComparator>
        m_plots;
      std::unordered_map<std::shared_ptr<ChartPlotSeries>,
        std::vector<RangeEntry>> m_ranges;
      ChartPlotView::AxisParameters m_xAxisParameters;
      Beam::ConnectionGroup m_seriesConnections;
      Beam::ConnectionGroup m_plotConnections;
      boost::signals2::scoped_connection m_xAxisParametersConnection;
      QTimer m_updateTimer;
      Beam::RoutineTaskQueue m_taskQueue;

      boost::optional<ChartValue> LoadLastValue();
      void UpdateAutoScale();
      void UpdateLockGrid();
      void QueryRange(const std::shared_ptr<ChartPlotSeries>& series,
        ChartValue min, ChartValue max);
      void OnChartPlotAdded(const std::shared_ptr<ChartPlot>& plot);
      void OnPlotUpdated(std::weak_ptr<ChartPlot> plot);
      void OnXAxisParametersChanged(
        const ChartPlotView::AxisParameters& parameters);
      void OnUpdateTimer();
  };
}

#endif
