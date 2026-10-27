#ifndef SPIRE_REPLAY_TIME_AND_SALES_MODEL_HPP
#define SPIRE_REPLAY_TIME_AND_SALES_MODEL_HPP
#include <deque>
#include <Beam/TimeService/TimeClient.hpp>
#include <QTimer>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Playback/PlaybackState.hpp"
#include "Spire/Playback/Timeline.hpp"
#include "Spire/Spire/Decimal.hpp"
#include "Spire/Spire/ListModelTransactionLog.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {
  class ReplaySession;

  /**
   * Implements a TimeAndSalesModel over a ticker's time and sales during a
   * replay, ending at its position and never earlier than its timeline's
   * start.
   */
  class ReplayTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /**
       * Constructs a ReplayTimeAndSalesModel.
       * @param ticker The ticker whose time and sales are replayed.
       * @param client The market data client used to query for time and sales.
       * @param session The ReplaySession followed, time and sales before its
       *        timeline's start are never listed.
       */
      ReplayTimeAndSalesModel(Nexus::Ticker ticker,
        Nexus::MarketDataClient client, std::shared_ptr<ReplaySession> session);

      QtPromise<void> load_older(int max_count) override;
      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
      using TimeAndSalesModel::transact;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      Nexus::Ticker m_ticker;
      Nexus::MarketDataClient m_client;
      std::shared_ptr<ReplaySession> m_session;
      Beam::TimeClient m_time_client;
      std::deque<Type> m_entries;
      ListModelTransactionLog<Type> m_transaction;
      std::deque<Type> m_buffer;
      boost::optional<Nexus::SequencedBboQuote> m_bbo;
      std::unique_ptr<EventHandler> m_live_handler;
      boost::posix_time::ptime m_position;
      boost::posix_time::ptime m_read_end;
      boost::posix_time::ptime m_last_read_time;
      boost::optional<Beam::Sequence> m_anchor;
      int m_page_size;
      int m_read_generation;
      int m_older_generation;
      bool m_is_reading;
      bool m_is_seeking;
      bool m_is_seek_querying;
      QTimer m_seek_timer;
      QtPromise<void> m_read_promise;
      QtPromise<void> m_seek_promise;
      boost::signals2::scoped_connection m_timeline_connection;
      boost::signals2::scoped_connection m_position_connection;
      boost::signals2::scoped_connection m_speed_connection;
      boost::signals2::scoped_connection m_state_connection;

      const boost::posix_time::ptime& get_start() const;
      Beam::Range::Point get_cursor() const;
      boost::posix_time::time_duration get_read_ahead() const;
      boost::posix_time::time_duration get_jump_threshold() const;
      int get_low_water() const;
      void append(Type entry);
      void pop_front();
      void pop_back();
      void replace(std::vector<Type> entries);
      bool merge(std::vector<Type>& entries);
      void publish();
      void read();
      void seek();
      void request_seek();
      void query_seek();
      void reconcile();
      void start_live();
      void stop_live();
      void on_read(int generation, boost::posix_time::ptime end,
        boost::optional<std::vector<Type>> entries,
        boost::optional<Nexus::SequencedBboQuote> bbo,
        boost::optional<Beam::Sequence> anchor);
      void on_seek(int generation, boost::posix_time::ptime position,
        boost::optional<std::vector<Type>> entries,
        boost::optional<Nexus::SequencedBboQuote> bbo,
        boost::optional<Beam::Sequence> anchor);
      void on_seek_timeout();
      void on_older(int generation, std::vector<Type> entries);
      void on_bbo(const Nexus::SequencedBboQuote& bbo);
      void on_time_and_sale(const Nexus::SequencedTimeAndSale& time_and_sale);
      void on_timeline(const Timeline& timeline);
      void on_position(const boost::posix_time::ptime& position);
      void on_speed(const Decimal& speed);
      void on_state(PlaybackState state);
  };
}

#endif
