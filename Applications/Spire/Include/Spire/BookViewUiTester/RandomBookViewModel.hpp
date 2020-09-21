#ifndef SPIRE_RANDOM_BOOK_VIEW_MODEL_HPP
#define SPIRE_RANDOM_BOOK_VIEW_MODEL_HPP
#include <random>
#include <Beam/Threading/CallOnce.hpp>
#include <QTimer>
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/LocalBookViewModel.hpp"
#include "Spire/BookViewUiTester/BookViewUiTester.hpp"

namespace Spire {

  //! Implements a book view model using randomly generated quotes.
  class RandomBookViewModel final : public BookViewModel, public QObject {
    public:

      //! Constructs a random book view model.
      /*!
        \param security The security to model.
        \param definitions The set of definitions to use.
        \param load_time The amount of time to take to load.
      */
      RandomBookViewModel(Nexus::Security security,
        Definitions definitions, boost::posix_time::time_duration load_time);

      //! Returns the update period.
      boost::posix_time::time_duration get_period() const;

      //! Sets the update period.
      void set_period(boost::posix_time::time_duration period);

      //! Publishes a book quote.
      void publish(const Nexus::BookQuote& quote);

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
      LocalBookViewModel m_model;
      boost::posix_time::time_duration m_load_time;
      boost::posix_time::time_duration m_period;
      std::default_random_engine m_random_engine;
      QTimer m_timer;
      std::shared_ptr<Beam::Threading::CallOnce<Beam::Threading::Mutex>>
        m_loader;

      Nexus::MarketCode get_random_market();
      void update();
      void update_bbo();
      void update_book_quote();
      void update_time_and_sales();
      void on_timeout();
  };
}

#endif
