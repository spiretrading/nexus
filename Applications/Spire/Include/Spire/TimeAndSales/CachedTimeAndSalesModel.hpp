#ifndef SPIRE_CACHED_TIME_AND_SALES_MODEL_HPP
#define SPIRE_CACHED_TIME_AND_SALES_MODEL_HPP
#include <memory>
#include <vector>
#include <boost/circular_buffer.hpp>
#include "Spire/Async/QtFuture.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /**
   * Implements a TimeAndSalesModel decorator that keeps the most recent entries
   * in a buffer so that a query for the latest entries can be returned
   * immediately.
   */
  class CachedTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /** The default number of recent entries to cache. */
      static const auto DEFAULT_COUNT = 100;

      /**
       * Constructs a CachedTimeAndSalesModel that caches a default number of
       * entries.
       * @param source The model to cache.
       */
      explicit CachedTimeAndSalesModel(
        std::shared_ptr<TimeAndSalesModel> source);

      /**
       * Constructs a CachedTimeAndSalesModel.
       * @param source The model to cache.
       * @param count The maximum number of recent entries to cache.
       */
      CachedTimeAndSalesModel(
        std::shared_ptr<TimeAndSalesModel> source, int count);

      QtPromise<std::vector<Entry>> query_until(
        Beam::Sequence sequence, int max_count) override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      enum class State {
        UNLOADED,
        LOADING,
        LOADED
      };
      struct PendingQuery {
        int m_max_count;
        QtFuture<std::vector<Entry>> m_result;
      };
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<TimeAndSalesModel> m_source;
      boost::circular_buffer<Entry> m_recent;
      std::vector<Entry> m_pending_updates;
      State m_state;
      std::vector<PendingQuery> m_pending;
      QtPromise<void> m_load;
      std::vector<QtPromise<void>> m_backfills;
      boost::signals2::scoped_connection m_source_connection;

      std::vector<Entry> get_recent(int max_count) const;
      QtPromise<std::vector<Entry>> load_older(int max_count);
      void on_update(const Entry& entry);
      void on_snapshot(std::vector<Entry> snapshot);
      void resolve_pending(PendingQuery query);
  };
}

#endif
