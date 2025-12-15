#ifndef SPIRE_AGGREGATE_BOOK_VIEW_MODEL_HPP
#define SPIRE_AGGREGATE_BOOK_VIEW_MODEL_HPP
#include "Spire/BookView/BookViewModel.hpp"

namespace Spire {

  /**
   * Implements a BookViewModel by aggregating all a BookViewModel's constituent
   * models.
   */
  class AggregateBookViewModel : public BookViewModel {
    public:

      /**
       * Constructs an AggregateBookViewModel by aggregating the provided
       * constituent models.
       * @param bids The list of bid quotes.
       * @param asks The list of ask quotes.
       * @param bid_orders The list of bid orders.
       * @param ask_orders The list of ask orders.
       * @param preview_order The preview order model.
       * @param bbo_quote The BBO quote model.
       * @param technicals The security technicals model.
       */
      AggregateBookViewModel(std::shared_ptr<BookQuoteListModel> bids,
        std::shared_ptr<BookQuoteListModel> asks,
        std::shared_ptr<UserOrderListModel> bid_orders,
        std::shared_ptr<UserOrderListModel> ask_orders,
        std::shared_ptr<PreviewOrderModel> preview_order,
        std::shared_ptr<BboQuoteModel> bbo_quote,
        std::shared_ptr<SecurityTechnicalsModel> technicals);

      const std::shared_ptr<BookQuoteListModel>& get_bids() const override;

      const std::shared_ptr<BookQuoteListModel>& get_asks() const override;

      const std::shared_ptr<UserOrderListModel>&
        get_bid_orders() const override;

      const std::shared_ptr<UserOrderListModel>&
        get_ask_orders() const override;

      const std::shared_ptr<PreviewOrderModel>&
        get_preview_order() const override;

      const std::shared_ptr<BboQuoteModel>& get_bbo_quote() const override;

      const std::shared_ptr<SecurityTechnicalsModel>&
        get_technicals() const override;

    private:
      std::shared_ptr<BookQuoteListModel> m_bids;
      std::shared_ptr<BookQuoteListModel> m_asks;
      std::shared_ptr<UserOrderListModel> m_bid_orders;
      std::shared_ptr<UserOrderListModel> m_ask_orders;
      std::shared_ptr<PreviewOrderModel> m_preview_order;
      std::shared_ptr<BboQuoteModel> m_bbo_quote;
      std::shared_ptr<SecurityTechnicalsModel> m_technicals;
  };

  /**
   * Builds an AggregateBookViewModel whose constituent models are made up of
   * LocalValueModel's or ArrayListModel's.
   */
  std::shared_ptr<AggregateBookViewModel>
    make_local_aggregate_book_view_model();
}

#endif
