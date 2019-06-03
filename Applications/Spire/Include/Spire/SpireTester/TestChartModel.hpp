#ifndef SPIRE_TEST_CHART_MODEL_HPP
#define SPIRE_TEST_CHART_MODEL_HPP
#include <deque>
#include <memory>
#include <Beam/Threading/ConditionVariable.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Spire/Charting/ChartModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

namespace Spire {

  //! Implements a ChartModel for testing purposes, allowing for the simulation
  //! of asynchronous loads.
  class TestChartModel final : public ChartModel {
    public:

      //! Stores a request to load a range of candlesticks.
      class LoadEntry {
        public:

          //! Constructs a LoadEntry.
          /*!
            \param first The beginning of the range to load.
            \param last The end of the range to load.
          */
          LoadEntry(ChartValue first, ChartValue end);

          //! Returns the beginning of the range to load.
          ChartValue get_first() const;

          //! Returns the end of the range to load.
          ChartValue get_last() const;

          //! Sets the result of the load operation.
          /*!
            \param result The list of candlesticks that the promise loading the
                   the specified range should evaluate to.
          */
          void set_result(std::vector<Candlestick> result);

          //! Returns the candlesticks to load.
          std::vector<Candlestick>& get_result();

        private:
          friend class TestChartModel;
          mutable Beam::Threading::Mutex m_mutex;
          ChartValue m_first;
          ChartValue m_last;
          bool m_is_loaded;
          std::vector<Candlestick> m_result;
          Beam::Threading::ConditionVariable m_load_condition;
      };

      //! Constructs a TestChartModel.
      /*!
        \param x_axis_type The type of x-axis values represented.
        \param x_axis_type The type of y-axis values represented.
      */
      TestChartModel(ChartValue::Type x_axis_type,
        ChartValue::Type y_axis_type);

      //! Pops the oldest load request from this model's load operation stack.
      std::shared_ptr<LoadEntry> pop_load();

      ChartValue::Type get_x_axis_type() const override;

      ChartValue::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(ChartValue first,
        ChartValue last) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      mutable CandlestickSignal m_candlestick_signal;
      ChartValue::Type m_x_axis_type;
      ChartValue::Type m_y_axis_type;
      std::deque<std::shared_ptr<LoadEntry>> m_load_entries;
  };
}

#endif
