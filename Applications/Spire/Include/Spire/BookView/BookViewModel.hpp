#ifndef SPIRE_BOOK_VIEW_MODEL_HPP
#define SPIRE_BOOK_VIEW_MODEL_HPP
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** A ValueModel over a BboQuote. */
  using BboQuoteModel = ValueModel<Nexus::BboQuote>;

  /** A LocalValueModel over a BboQuote. */
  using LocalBboQuoteModel = LocalValueModel<Nexus::BboQuote>;

  /** A ValueModel over a SecurityTechnicals. */
  using SecurityTechnicalsValueModel = ValueModel<Nexus::SecurityTechnicals>;

  /** A LocalValueModel over a SecurityTechnicals. */
  using LocalSecurityTechnicalsValueModel =
    LocalValueModel<Nexus::SecurityTechnicals>;

  /** The model for the book view. */
  class BookViewModel {
    public:

      /** A ValueModel over optional OrderFields. */
      using PreviewOrderModel =
        ValueModel<boost::optional<Nexus::OrderExecutionService::OrderFields>>;

      /** Represents the user order. */
      struct UserOrder {

        /** The order destination. */
        Nexus::Destination m_destination;

        /** The order price. */
        Nexus::Money m_price;

        /** The order size. */
        Nexus::Quantity m_size;

        /** The status of the user order. */
        Nexus::OrderStatus m_status;
      };

      /* Constructs a BookViewModel. */
      BookViewModel();

      /** Returns a list of BookQuotes with the bid side. */
      const std::shared_ptr<ListModel<Nexus::BookQuote>>& get_bids() const;

      /** Returns a list of BookQuotes with the ask side. */
      const std::shared_ptr<ListModel<Nexus::BookQuote>>& get_asks() const;

      /** Returns a list of orders with the bid side. */
      const std::shared_ptr<ListModel<UserOrder>>& get_bid_orders() const;

      /** Returns a list of orders with the ask side. */
      const std::shared_ptr<ListModel<UserOrder>>& get_ask_orders() const;

      /** Returns the preview order. */
      const std::shared_ptr<PreviewOrderModel>& get_preview_order() const;

      /** Returns the Bbo quote. */
      const std::shared_ptr<BboQuoteModel>& get_bbo_quote() const;

      /** Returns the technical details about a Security. */
      const std::shared_ptr<SecurityTechnicalsValueModel>& get_technicals()
        const;

    private:
      std::shared_ptr<ListModel<Nexus::BookQuote>> m_bids;
      std::shared_ptr<ListModel<Nexus::BookQuote>> m_asks;
      std::shared_ptr<ListModel<UserOrder>> m_bid_orders;
      std::shared_ptr<ListModel<UserOrder>> m_ask_orders;
      std::shared_ptr<PreviewOrderModel> m_preview_order;
      std::shared_ptr<BboQuoteModel> m_bbo;
      std::shared_ptr<SecurityTechnicalsValueModel> m_technicals;
  };
}

#endif
