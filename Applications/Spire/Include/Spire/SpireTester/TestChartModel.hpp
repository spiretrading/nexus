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
            \param limit The limit on the candlesticks to load.
          */
          LoadEntry(Scalar first, Scalar end,
            const Beam::Queries::SnapshotLimit& limit);

          //! Returns the beginning of the range to load.
          Scalar get_first() const;

          //! Returns the end of the range to load.
          Scalar get_last() const;

          //! Returns the limit.
          const Beam::Queries::SnapshotLimit& get_limit() const;

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
          Scalar m_first;
          Scalar m_last;
          Beam::Queries::SnapshotLimit m_limit;
          bool m_is_loaded;
          std::vector<Candlestick> m_result;
          Beam::Threading::ConditionVariable m_load_condition;
      };

      //! Constructs a TestChartModel.
      /*!
        \param x_axis_type The type of x-axis values represented.
        \param x_axis_type The type of y-axis values represented.
      */
      TestChartModel(Scalar::Type x_axis_type, Scalar::Type y_axis_type);

      //! Pops the oldest load request from this model's load operation stack.
      QtPromise<std::shared_ptr<LoadEntry>> pop_load();

      Scalar::Type get_x_axis_type() const override;

      Scalar::Type get_y_axis_type() const override;

      QtPromise<std::vector<Candlestick>> load(Scalar first, Scalar last,
        const Beam::Queries::SnapshotLimit& limit) override;

      boost::signals2::connection connect_candlestick_slot(
        const CandlestickSignal::slot_type& slot) const override;

    private:
      Beam::Threading::Mutex m_mutex;
      Beam::Threading::ConditionVariable m_load_condition;
      mutable CandlestickSignal m_candlestick_signal;
      Scalar::Type m_x_axis_type;
      Scalar::Type m_y_axis_type;
      std::deque<std::shared_ptr<LoadEntry>> m_load_entries;
  };
}

#endif
