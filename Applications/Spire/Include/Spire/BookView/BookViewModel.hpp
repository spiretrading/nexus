#ifndef SPIRE_BOOK_VIEW_MODEL_HPP
#define SPIRE_BOOK_VIEW_MODEL_HPP
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

namespace Spire {

  /** A ValueModel over a BboQuote. */
  using BboQuoteModel = ValueModel<Nexus::BboQuote>;

  /** A LocalValueModel over a BboQuote. */
  using LocalBboQuoteModel = LocalValueModel<Nexus::BboQuote>;

  /** Models a list of BookQuotes. */
  using BookQuoteListModel = ListModel<Nexus::BookQuote>;

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

        bool operator ==(const UserOrder&) const = default;
      };

      /** Models a list of UserOrders. */
      using UserOrderListModel = ListModel<UserOrder>;

      virtual ~BookViewModel() = default;

      /** Returns a list of BookQuotes with the bid side. */
      virtual const std::shared_ptr<BookQuoteListModel>& get_bids() const = 0;

      /** Returns a list of BookQuotes with the ask side. */
      virtual const std::shared_ptr<BookQuoteListModel>& get_asks() const = 0;

      /** Returns a list of orders with the bid side. */
      virtual const std::shared_ptr<UserOrderListModel>&
        get_bid_orders() const = 0;

      /** Returns a list of orders with the ask side. */
      virtual const std::shared_ptr<UserOrderListModel>&
        get_ask_orders() const = 0;

      /** Returns the preview order. */
      virtual const std::shared_ptr<PreviewOrderModel>&
        get_preview_order() const = 0;

      /** Returns the Bbo quote. */
      virtual const std::shared_ptr<BboQuoteModel>& get_bbo_quote() const = 0;

      /** Returns the technical details about a Security. */
      virtual const std::shared_ptr<SecurityTechnicalsValueModel>&
        get_technicals() const = 0;

    protected:

      /** Constructs an empty model. */
      BookViewModel() = default;

    private:
      BookViewModel(const BookViewModel&) = delete;
      BookViewModel& operator =(const BookViewModel&) = delete;
  };
}

#endif
