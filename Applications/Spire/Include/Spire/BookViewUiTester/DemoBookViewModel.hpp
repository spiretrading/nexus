#ifndef SPIRE_DEMO_BOOK_VIEW_MODEL_HPP
#define SPIRE_DEMO_BOOK_VIEW_MODEL_HPP
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/BookView/BookViewModel.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/Spire/ListModel.hpp"

namespace Spire {

  /** The model for testing the book view. */
  class DemoBookViewModel {
    public:

      /** Stores details about an Order. */
      struct OrderInfo {

        /** The Order details. */
        Nexus::OrderExecutionService::OrderFields m_order_fields;

        /** The Order status. */
        Nexus::OrderStatus m_status;
      };

      /* Constructs a DemoBookViewModel. */
      explicit DemoBookViewModel(std::shared_ptr<BookViewModel> model);

      /** Returns the book view model. */
      const std::shared_ptr<BookViewModel>& get_model() const;

      /** Submits an book quote. */
      void submit_book_quote(const Nexus::BookQuote& quote);

      /** Submits an order. */
      void submit_order(const OrderInfo& order);

    private:
      std::shared_ptr<BookViewModel> m_model;
      Nexus::BboQuote m_bbo;
      boost::signals2::scoped_connection m_bid_operation_connection;
      boost::signals2::scoped_connection m_ask_operation_connection;

      void on_bid_operation(
        const ListModel<Nexus::BookQuote>::Operation& operation);
      void on_ask_operation(
        const ListModel<Nexus::BookQuote>::Operation& operation);
  };
}

#endif
