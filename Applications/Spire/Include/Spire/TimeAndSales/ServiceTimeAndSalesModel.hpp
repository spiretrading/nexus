#ifndef SPIRE_SERVICE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_SERVICE_TIME_AND_SALES_MODEL_HPP
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"

namespace Spire {

  /** Implements the TimeAndSalesModel using remote service calls. */
  class ServiceTimeAndSalesModel : public TimeAndSalesModel {
    public:

      /**
       * Constructs a ServiceTimeAndSalesModel with 50 entries initially loaded.
       * @param security The security whose time and sales are queried.
       * @param client The market data client used to query for time and sales.
       */
      ServiceTimeAndSalesModel(
        Nexus::Security security, Nexus::MarketDataClient client);

      QtPromise<std::vector<Entry>> query_until(
        Beam::Sequence sequence, int max_count) override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      Nexus::Security m_security;
      Nexus::MarketDataClient m_client;
      Nexus::SequencedBboQuote m_bbo;
      EventHandler m_event_handler;

      void on_bbo(const Nexus::SequencedBboQuote& bbo);
      void on_time_and_sale(const Nexus::SequencedTimeAndSale& time_and_sale);
  };
}

#endif
