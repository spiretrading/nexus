#ifndef SPIRE_EMPTY_BOOK_VIEW_MODEL_HPP
#define SPIRE_EMPTY_BOOK_VIEW_MODEL_HPP
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  //! Implements the BookViewModel with no quotes.
  class EmptyBookViewModel final : public BookViewModel {
    public:

      //! Constructs an empty book view model.
      /*!
        \param security The security to model.
      */
      explicit EmptyBookViewModel(Nexus::Security security);

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
      Nexus::Security m_security;
  };
}

#endif
