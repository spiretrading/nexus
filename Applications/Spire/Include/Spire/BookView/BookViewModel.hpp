#ifndef SPIRE_BOOK_VIEW_MODEL_HPP
#define SPIRE_BOOK_VIEW_MODEL_HPP
#include <vector>
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** The model for the book view. */
  class BookViewModel {
    public:

      /** Represents the user order. */
      struct UserOrder {

        /** The order destination. */
        std::string m_destination;
        
        /** The order price. */
        Nexus::Money m_price;

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

    private:
      std::shared_ptr<ListModel<Nexus::BookQuote>> m_bids;
      std::shared_ptr<ListModel<Nexus::BookQuote>> m_asks;
      std::shared_ptr<ListModel<UserOrder>> m_bid_orders;
      std::shared_ptr<ListModel<UserOrder>> m_ask_orders;
  };
}

#endif
