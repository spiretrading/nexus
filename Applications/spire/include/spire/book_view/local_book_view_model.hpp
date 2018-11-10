#ifndef SPIRE_LOCAL_BOOK_VIEW_MODEL_HPP
#define SPIRE_LOCAL_BOOK_VIEW_MODEL_HPP
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_model.hpp"

namespace Spire {

  //! Implements a programmatic BookViewModel in memory.
  class LocalBookViewModel final : public BookViewModel {
    public:

      //! Constructs an empty model.
      /*!
        \param security The security to model.
      */
      LocalBookViewModel(Nexus::Security security);

      //! Updates the BBO.
      /*!
        \param bbo The updated BBO.
      */
      void update(const Nexus::BboQuote& bbo);

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
      Nexus::BboQuote m_bbo;
      std::vector<Nexus::BookQuote> m_asks;
      std::vector<Nexus::BookQuote> m_bids;
      boost::optional<Nexus::Money> m_high;
      boost::optional<Nexus::Money> m_low;
      boost::optional<Nexus::Money> m_open;
      boost::optional<Nexus::Money> m_close;
      Nexus::Quantity m_volume;
  };
}

#endif
