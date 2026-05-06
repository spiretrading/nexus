#ifndef SPIRE_LOCAL_BOOK_VIEW_MODEL_HPP
#define SPIRE_LOCAL_BOOK_VIEW_MODEL_HPP
#include <functional>
#include <unordered_map>
#include <vector>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/BookView/AggregateBookViewModel.hpp"

namespace Spire {

  /** Implements a BookViewModel with methods to update its state locally. */
  class LocalBookViewModel : public BookViewModel {
    public:

      /** Constructs a LocalBookViewModel. */
      LocalBookViewModel();

      /** Updates the BBO quote. */
      void update(const Nexus::BboQuote& bbo);

      /** Updates a book quote. */
      void update(const Nexus::BookQuote& quote);

      /** Updates from a time and sale. */
      void update(const Nexus::TimeAndSale& time_and_sale);

      /** Adds an order. */
      void add(const OrderLogModel::OrderEntry& order);

      /**
       * Adds an order with a specified quantity and status.
       * @param order The order to add.
       * @param quantity The remaining quantity.
       * @param status The order's status.
       */
      void add(const OrderLogModel::OrderEntry& order, Nexus::Quantity quantity,
        Nexus::OrderStatus status);

      /** Removes an order. */
      void remove(const OrderLogModel::OrderEntry& order);

      /** Updates from an execution report. */
      void update(const Nexus::ExecutionReport& report);

      /** Removes all orders and pegged entries. */
      void clear_orders();

      /** Removes all book quotes. */
      void clear_book_quotes();

      /** Performs a transaction. */
      void transact(const std::function<void ()>& f);

      const std::shared_ptr<BookQuoteListModel>& get_bids() const override;
      const std::shared_ptr<BookQuoteListModel>& get_asks() const override;
      const std::shared_ptr<UserOrderListModel>&
        get_bid_orders() const override;
      const std::shared_ptr<UserOrderListModel>&
        get_ask_orders() const override;
      const std::shared_ptr<PreviewOrderModel>&
        get_preview_order() const override;
      const std::shared_ptr<BboQuoteModel>& get_bbo_quote() const override;
      const std::shared_ptr<SessionCandlestickModel>&
        get_session_candlestick() const override;

    private:
      struct PeggedOrderEntry {
        std::string m_exec_inst;
        Nexus::Money m_peg_difference;
        Nexus::Money m_effective_price;
      };
      AggregateBookViewModel m_model;
      std::vector<std::shared_ptr<Nexus::Order>> m_bid_orders;
      std::vector<std::shared_ptr<Nexus::Order>> m_ask_orders;
      std::unordered_map<Nexus::OrderId, PeggedOrderEntry> m_pegged_entries;

      void submit_pegged(const Nexus::Order& order);
      void update_pegged_orders();
  };
}

#endif
