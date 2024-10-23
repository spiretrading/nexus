#ifndef SPIRE_DEMO_BOOK_VIEW_MODEL_HPP
#define SPIRE_DEMO_BOOK_VIEW_MODEL_HPP
#include "Spire/BookView/BookViewModel.hpp"

namespace Spire {

  /** The the book view model for the demo. */
  class DemoBookViewModel : public BookViewModel {
    public:

      using QuoteSignal = typename BookViewModel::QuoteSignal;
      using BboSignal = typename BookViewModel::BboSignal;
      using SecurityTechnicalsSignal = typename BookViewModel::SecurityTechnicalsSignal;
      using DefaultQuantitySignal = typename BookViewModel::DefaultQuantitySignal;
      using OrderExecutionReportSignal = typename BookViewModel::OrderExecutionReportSignal;

      /* Constructs a DemoBookViewModel. */
      DemoBookViewModel();

      const std::vector<const OrderExecutionService::Order*> get_orders() const override;

      void submit_order(const OrderFields& order) override;

      void cancel_orders(const std::vector<OrderFields>& orders) override;

      boost::signals2::connection connect_quote_signal(
        const QuoteSignal::slot_type& slot) const override;

      boost::signals2::connection connect_bbo_signal(
        const BboSignal::slot_type& slot) const override;

      boost::signals2::connection connect_security_technicals_signal(
        const SecurityTechnicalsSignal::slot_type& slot) const override;

      boost::signals2::connection connect_default_quantity_signal(
        const DefaultQuantitySignal::slot_type& slot) const override;

      boost::signals2::connection connect_order_execution_report_signal(
        const OrderExecutionReportSignal::slot_type& slot) const override;

  };
}

#endif
