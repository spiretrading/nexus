#ifndef NEXUS_ORDER_TO_BBO_QUOTE_MODEL_HPP
#define NEXUS_ORDER_TO_BBO_QUOTE_MODEL_HPP
#include <algorithm>
#include <iterator>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/OrderToBookQuoteModel.hpp"

namespace Nexus {

  /**
   * Maintains a BboQuote from order updates.
   * @tparam A Adapts original orders to book quotes.
   */
  template<IsOrderQuoteAdapter A>
  class OrderToBboQuoteModel {
    public:

      /** Converts original orders to book quotes. */
      using Adapter = A;

      /** The type used to identify orders. */
      using OrderId = typename Adapter::OrderId;

      /** The original order stored by the model. */
      using Order = typename Adapter::Order;

      /** Maintains the orders and aggregate quotes for one side. */
      using Book = OrderToBookQuoteModel<Adapter>;

      /** The quote changes produced by an order operation. */
      struct Update {

        /** The updated aggregate quotes. */
        typename Book::Updates m_quotes;

        /** Whether the BboQuote changed. */
        bool m_is_bbo_changed;
      };

      OrderToBboQuoteModel() requires std::default_initializable<Adapter>;

      /** Constructs a model using an order adapter. */
      explicit OrderToBboQuoteModel(Adapter adapter);

      /** Returns the adapter used to produce quotes. */
      const Adapter& get_adapter() const;

      /**
       * Replaces the adapter and refreshes the quotes.
       * @param adapter The new order adapter.
       * @param timestamp The update's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update set_adapter(Adapter adapter, boost::posix_time::ptime timestamp);

      /** Returns the current BboQuote. */
      const BboQuote& get_bbo() const;

      /**
       * Returns the order book for a side.
       * @param side The side to inspect.
       */
      const Book& get_book(Side side) const;

      /**
       * Adds or replaces an original order on a side.
       * @param side The order's side.
       * @param id The order's identifier.
       * @param order The original order.
       * @param timestamp The update's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update add(Side side, const OrderId& id, Order order,
        boost::posix_time::ptime timestamp);

      /**
       * Adds or replaces an order.
       * @param id The order's identifier.
       * @param order The individual order's quote.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update add(const OrderId& id, BookQuote order) requires
        std::same_as<Order, BookQuote>;

      /**
       * Updates an original order on a side.
       * @param side The order's side.
       * @param id The order's identifier.
       * @param f The operation applied to the order.
       * @param timestamp The update's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      template<std::invocable<typename A::Order&> F>
      Update update(
        Side side, const OrderId& id, F f, boost::posix_time::ptime timestamp);

      /**
       * Refreshes all quotes using the current adapter.
       * @param timestamp The update's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update refresh(boost::posix_time::ptime timestamp);

      /**
       * Sets an order's size, removing it if the size is nonpositive.
       * @param id The order's identifier.
       * @param size The new size.
       * @param timestamp The update's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update modify_size(
        const OrderId& id, Quantity size,
        boost::posix_time::ptime timestamp) requires
        std::same_as<Order, BookQuote>;

      /**
       * Adjusts an order's size.
       * @param id The order's identifier.
       * @param delta The change in size.
       * @param timestamp The update's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update offset_size(
        const OrderId& id, Quantity delta,
        boost::posix_time::ptime timestamp) requires
        std::same_as<Order, BookQuote>;

      /**
       * Sets an order's price.
       * @param id The order's identifier.
       * @param price The new price.
       * @param timestamp The update's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update modify_price(
        const OrderId& id, Money price,
        boost::posix_time::ptime timestamp) requires
        std::same_as<Order, BookQuote>;

      /**
       * Removes an order from a side.
       * @param side The order's side.
       * @param id The order's identifier.
       * @param timestamp The removal's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update remove(
        Side side, const OrderId& id, boost::posix_time::ptime timestamp);

      /**
       * Removes an order.
       * @param id The order's identifier.
       * @param timestamp The removal's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update remove(const OrderId& id, boost::posix_time::ptime timestamp);

      /**
       * Removes all orders.
       * @param timestamp The removal's timestamp.
       * @return The aggregate quote changes and whether the BboQuote changed.
       */
      Update clear(boost::posix_time::ptime timestamp);

    private:
      using Updates = typename Book::Updates;
      Book m_bids;
      Book m_asks;
      BboQuote m_bbo;

      Update update(
        Side side, Updates quotes, boost::posix_time::ptime timestamp);
      Update update(
        Updates bids, Updates asks, boost::posix_time::ptime timestamp);
      bool update(const Book& book, const Updates& updates);
  };

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel(A) -> OrderToBboQuoteModel<A>;

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::OrderToBboQuoteModel() requires
    std::default_initializable<Adapter>
    : OrderToBboQuoteModel(Adapter()) {}

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::OrderToBboQuoteModel(Adapter adapter)
    : m_bids(Side::BID, adapter),
      m_asks(Side::ASK, std::move(adapter)) {}

  template<IsOrderQuoteAdapter A>
  const OrderToBboQuoteModel<A>::Adapter&
      OrderToBboQuoteModel<A>::get_adapter() const {
    return m_bids.get_adapter();
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::set_adapter(
      Adapter adapter, boost::posix_time::ptime timestamp) {
    auto bids = m_bids.set_adapter(adapter, timestamp);
    auto asks = m_asks.set_adapter(std::move(adapter), timestamp);
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  const BboQuote& OrderToBboQuoteModel<A>::get_bbo() const {
    return m_bbo;
  }

  template<IsOrderQuoteAdapter A>
  const OrderToBboQuoteModel<A>::Book&
      OrderToBboQuoteModel<A>::get_book(Side side) const {
    return pick(side, m_asks, m_bids);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::add(Side side,
      const OrderId& id, Order order, boost::posix_time::ptime timestamp) {
    auto& book = pick(side, m_asks, m_bids);
    return update(side, book.add(id, std::move(order), timestamp), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::add(
      const OrderId& id, BookQuote order) requires
      std::same_as<Order, BookQuote> {
    auto timestamp = order.m_timestamp;
    auto bids = Updates();
    auto asks = Updates();
    if(order.m_quote.m_side == Side::BID) {
      bids = m_bids.add(id, std::move(order));
    } else {
      asks = m_asks.add(id, std::move(order));
    }
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  template<std::invocable<typename A::Order&> F>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::update(Side side,
      const OrderId& id, F f, boost::posix_time::ptime timestamp) {
    auto& book = pick(side, m_asks, m_bids);
    return update(side, book.update(id, std::move(f), timestamp), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::refresh(
      boost::posix_time::ptime timestamp) {
    auto bids = m_bids.refresh(timestamp);
    auto asks = m_asks.refresh(timestamp);
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::modify_size(
      const OrderId& id, Quantity size,
      boost::posix_time::ptime timestamp) requires
      std::same_as<Order, BookQuote> {
    auto bids = m_bids.modify_size(id, size, timestamp);
    auto asks = m_asks.modify_size(id, size, timestamp);
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::offset_size(
      const OrderId& id, Quantity delta,
      boost::posix_time::ptime timestamp) requires
      std::same_as<Order, BookQuote> {
    auto bids = m_bids.offset_size(id, delta, timestamp);
    auto asks = m_asks.offset_size(id, delta, timestamp);
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::modify_price(
      const OrderId& id, Money price,
      boost::posix_time::ptime timestamp) requires
      std::same_as<Order, BookQuote> {
    auto bids = m_bids.modify_price(id, price, timestamp);
    auto asks = m_asks.modify_price(id, price, timestamp);
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::remove(
      Side side, const OrderId& id, boost::posix_time::ptime timestamp) {
    auto& book = pick(side, m_asks, m_bids);
    return update(side, book.remove(id, timestamp), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::remove(
      const OrderId& id, boost::posix_time::ptime timestamp) {
    auto bids = m_bids.remove(id, timestamp);
    auto asks = m_asks.remove(id, timestamp);
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::clear(
      boost::posix_time::ptime timestamp) {
    auto bids = m_bids.clear(timestamp);
    auto asks = m_asks.clear(timestamp);
    return update(std::move(bids), std::move(asks), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::update(
      Side side, Updates quotes, boost::posix_time::ptime timestamp) {
    if(side == Side::BID) {
      return update(std::move(quotes), Updates(), timestamp);
    }
    return update(Updates(), std::move(quotes), timestamp);
  }

  template<IsOrderQuoteAdapter A>
  OrderToBboQuoteModel<A>::Update OrderToBboQuoteModel<A>::update(
      Updates bids, Updates asks, boost::posix_time::ptime timestamp) {
    auto is_bid_changed = update(m_bids, bids);
    auto is_ask_changed = update(m_asks, asks);
    auto is_bbo_changed = is_bid_changed || is_ask_changed;
    if(is_bbo_changed) {
      m_bbo.m_timestamp = timestamp;
    }
    if(bids.empty()) {
      return Update(std::move(asks), is_bbo_changed);
    }
    bids.insert(bids.end(), std::make_move_iterator(asks.begin()),
      std::make_move_iterator(asks.end()));
    return Update(std::move(bids), is_bbo_changed);
  }

  template<IsOrderQuoteAdapter A>
  bool OrderToBboQuoteModel<A>::update(
      const Book& book, const Updates& updates) {
    if(updates.empty()) {
      return false;
    }
    auto side = book.get_side();
    auto& bbo = pick(side, m_bbo.m_ask, m_bbo.m_bid);
    auto affects_best = [&] (const auto& quote) {
      return offer_comparator(side, quote.m_quote.m_price, bbo.m_price) <= 0;
    };
    if(bbo.m_size != 0 && std::ranges::none_of(updates, affects_best)) {
      return false;
    }
    auto quote = Quote(Money::ZERO, 0, side);
    if(!book.empty()) {
      quote.m_price = book[0].m_quote.m_price;
      for(auto& entry : book) {
        if(entry.m_quote.m_price != quote.m_price) {
          break;
        }
        quote.m_size += entry.m_quote.m_size;
      }
    }
    if(quote == bbo) {
      return false;
    }
    bbo = quote;
    return true;
  }
}

#endif
