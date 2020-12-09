#ifndef SPIRE_SERVICES_BOOK_VIEW_MODEL_HPP
#define SPIRE_SERVICES_BOOK_VIEW_MODEL_HPP
#include <memory>
#include <Beam/Threading/CallOnce.hpp>
#include <Beam/Threading/Mutex.hpp>
#include "Nexus/ServiceClients/ServiceClientsBox.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/LocalBookViewModel.hpp"
#include "Spire/Spire/Definitions.hpp"
#include "Spire/Spire/EventHandler.hpp"

namespace Spire {

  //! Implements the BookViewModel using remote service calls.
  class ServicesBookViewModel final : public BookViewModel {
    public:

      //! Constructs a model.
      /*!
        \param security The security to model.
        \param definitions The set of definitions.
        \param clients The service clients to query for market data.
      */
      ServicesBookViewModel(Nexus::Security security,
        Definitions definitions, Nexus::ServiceClientsBox clients);

      const Nexus::Security& get_security() const override;

      const Nexus::BboQuote& get_bbo() const override;

      const std::vector<std::unique_ptr<Quote>>& get_asks() const override;

      const std::vector<std::unique_ptr<Quote>>& get_bids() const override;

      boost::optional<Nexus::Money> get_high() const override;

      boost::optional<Nexus::Money> get_low() const override;

      boost::optional<Nexus::Money> get_open() const override;

      boost::optional<Nexus::Money> get_close() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<void> load() override;

      const Nexus::MarketDatabase& get_market_database() const;

      boost::signals2::connection connect_bbo_slot(
        const BboSignal::slot_type& slot) const override;

      boost::signals2::connection connect_quote_slot(
        const QuoteSignal::slot_type& slot) const override;

      boost::signals2::connection connect_high_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_low_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_open_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_close_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_slot(
        const QuantitySignal::slot_type& slot) const override;

    private:
      LocalBookViewModel m_local_model;
      Definitions m_definitions;
      Nexus::ServiceClientsBox m_clients;
      std::shared_ptr<Beam::Threading::CallOnce<Beam::Threading::Mutex>>
        m_loader;
      EventHandler m_event_handler;

      void on_bbo(const Nexus::BboQuote& quote);
      void on_book_quote(const Nexus::BookQuote& quote);
      void on_book_quote_interruption(const std::exception_ptr& e);
      void on_market_quote(const Nexus::MarketQuote& quote);
      void on_market_quote_interruption(const std::exception_ptr& e);
      void on_volume(const Nexus::Queries::QueryVariant& value);
      void on_high(const Nexus::Queries::QueryVariant& value);
      void on_low(const Nexus::Queries::QueryVariant& value);
      void on_open(const Nexus::TimeAndSale& value);
      void on_close(const Nexus::TimeAndSale& value);
  };
}

#endif
