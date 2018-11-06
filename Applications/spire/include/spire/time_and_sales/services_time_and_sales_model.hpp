#ifndef SPIRE_SERVICES_TIME_AND_SALES_MODEL_HPP
#define SPIRE_SERVICES_TIME_AND_SALES_MODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"

namespace Spire {

  //! Implements a time and sales model using remote service calls.
  class ServicesTimeAndSalesModel final : public TimeAndSalesModel {
    public:

      //! Constructs a model.
      /*!
        \param security The security to model.
        \param client The market data client to query.
      */
      ServicesTimeAndSalesModel(Nexus::Security security,
        Beam::Ref<Nexus::MarketDataService::VirtualMarketDataClient> client);

      const Nexus::Security& get_security() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<std::vector<Entry>> load_snapshot(Beam::Queries::Sequence last,
        int count) override;

      boost::signals2::connection connect_time_and_sale_signal(
        const TimeAndSaleSignal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_signal(
        const VolumeSignal::slot_type& slot) const override;

    private:
      Nexus::Security m_security;
      Nexus::MarketDataService::VirtualMarketDataClient* m_client;
  };
}

#endif
