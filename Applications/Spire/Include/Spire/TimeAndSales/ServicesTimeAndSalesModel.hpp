#ifndef SPIRE_SERVICES_TIME_AND_SALES_MODEL_HPP
#define SPIRE_SERVICES_TIME_AND_SALES_MODEL_HPP
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/Spire/EventHandler.hpp"

namespace Spire {

  //! Implements the TimeAndSalesModel using remote service calls.
  class ServicesTimeAndSalesModel final : public TimeAndSalesModel {
    public:

      //! Constructs a model.
      /*!
        \param security The security to model.
        \param definitions The set of definitions.
        \param clients The service clients to query for market data.
      */
      ServicesTimeAndSalesModel(Nexus::Security security,
        const Definitions& definitions, Nexus::ServiceClientsBox clients);

      const Nexus::Security& get_security() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<std::vector<Entry>> load_snapshot(Beam::Queries::Sequence last,
        int count) override;

      boost::signals2::connection connect_time_and_sale_signal(
        const TimeAndSaleSignal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_signal(
        const VolumeSignal::slot_type& slot) const override;

    private:
      mutable TimeAndSaleSignal m_time_and_sale_signal;
      mutable VolumeSignal m_volume_signal;
      Nexus::Security m_security;
      Nexus::ServiceClientsBox m_clients;
      Nexus::SequencedBboQuote m_bbo;
      Nexus::Quantity m_volume;
      EventHandler m_event_handler;

      void on_bbo(const Nexus::SequencedBboQuote& bbo);
      void on_time_and_sale(const Nexus::SequencedTimeAndSale& time_and_sale);
  };
}

#endif
