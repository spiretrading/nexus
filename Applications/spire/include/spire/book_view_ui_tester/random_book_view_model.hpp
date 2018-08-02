#ifndef SPIRE_RANDOM_BOOK_VIEW_MODEL_HPP
#define SPIRE_RANDOM_BOOK_VIEW_MODEL_HPP
#include <random>
#include <Beam/Threading/CallOnce.hpp>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <QTimer>
#include "spire/book_view_ui_tester/book_view_ui_tester.hpp"
#include "spire/book_view/book_view_model.hpp"

namespace spire {

  //! Implements a book view model using randomly generated quotes.
  class RandomBookViewModel final : public BookViewModel, public QObject {
    public:

      //! Constructs a random book view model.
      /*!
        \param security The security to model.
        \param load_time The amount of time to take to load.
      */
      RandomBookViewModel(Nexus::Security security,
        boost::posix_time::time_duration load_time,
        Beam::Threading::TimerThreadPool& timer_thread_pool);

      //! Returns the update period.
      boost::posix_time::time_duration get_period() const;

      //! Sets the update period.
      void set_period(boost::posix_time::time_duration period);

      const Nexus::Security& get_security() const override;

      const Nexus::BboQuote& get_bbo() const override;

      const std::vector<Nexus::BookQuote>& get_asks() const override;

      const std::vector<Nexus::BookQuote>& get_bids() const override;

      boost::optional<Nexus::Money> get_high() const override;

      boost::optional<Nexus::Money> get_low() const override;

      boost::optional<Nexus::Money> get_open() const override;

      boost::optional<Nexus::Money> get_close() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<void> load() override;

      boost::signals2::connection connect_bbo_slot(
        const BboSignal::slot_type& slot) const override;

      boost::signals2::connection connect_book_quote_slot(
        const BookQuoteSignal::slot_type& slot) const override;

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
      mutable BboSignal m_bbo_signal;
      mutable BookQuoteSignal m_book_quote_signal;
      mutable PriceSignal m_high_signal;
      mutable PriceSignal m_low_signal;
      mutable PriceSignal m_open_signal;
      mutable PriceSignal m_close_signal;
      mutable QuantitySignal m_volume_signal;
      Nexus::Security m_security;
      boost::posix_time::time_duration m_load_time;
      Beam::Threading::TimerThreadPool* m_timer_thread_pool;
      boost::posix_time::time_duration m_period;
      Nexus::BboQuote m_bbo;
      std::vector<Nexus::BookQuote> m_asks;
      std::vector<Nexus::BookQuote> m_bids;
      Nexus::Money m_high;
      Nexus::Money m_low;
      Nexus::Money m_open;
      Nexus::Money m_close;
      Nexus::Quantity m_volume;
      std::default_random_engine m_random_engine;
      QTimer m_timer;
      Beam::Threading::CallOnce<Beam::Threading::Mutex> m_loading_flag;

      void update();
      void on_timeout();
  };
}

#endif
