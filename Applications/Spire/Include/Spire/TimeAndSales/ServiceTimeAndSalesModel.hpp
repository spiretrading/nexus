#ifndef SPIRE_SERVICE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_SERVICE_TIME_AND_SALES_MODEL_HPP
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /** Implements the TimeAndSalesModel using remote service calls. */
  class ServiceTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /**
       * Constructs a ServiceTimeAndSalesModel.
       * @param ticker The ticker whose time and sales are queried.
       * @param client The market data client used to query for time and sales.
       */
      ServiceTimeAndSalesModel(
        Nexus::Ticker ticker, Nexus::MarketDataClient client);

      QtPromise<void> load_older(int max_count) override;
      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      Nexus::Ticker m_ticker;
      Nexus::MarketDataClient m_client;
      ArrayListModel<Details::TimeAndSalesEntry> m_entries;
      Nexus::SequencedBboQuote m_bbo;
      EventHandler m_event_handler;

      void on_bbo(const Nexus::SequencedBboQuote& bbo);
      void on_time_and_sale(const Nexus::SequencedTimeAndSale& time_and_sale);
  };
}

#endif
