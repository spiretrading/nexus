#ifndef NEXUS_ORDER_TO_BBO_QUOTE_MODEL_HPP
#define NEXUS_ORDER_TO_BBO_QUOTE_MODEL_HPP
#include <algorithm>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/OrderToBookQuoteModel.hpp"

namespace Nexus {

  /**
   * Maintains a BboQuote from order updates.
   * @tparam O The type used to identify orders.
   */
  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  class OrderToBboQuoteModel {
    public:

      /** The type used to identify orders. */
      using OrderId = O;

      OrderToBboQuoteModel();

      /** Returns the current BboQuote. */
      const BboQuote& get_bbo() const;

      /**
       * Adds or replaces an order.
       * @param id The order's identifier.
       * @param order The individual order's quote.
       * @return Whether the BboQuote changed.
       */
      bool add(const OrderId& id, BookQuote order);

      /**
       * Sets an order's size, removing it if the size is nonpositive.
       * @param id The order's identifier.
       * @param size The new size.
       * @param timestamp The update's timestamp.
       * @return Whether the BboQuote changed.
       */
      bool modify_size(
        const OrderId& id, Quantity size, boost::posix_time::ptime timestamp);

      /**
       * Adjusts an order's size.
       * @param id The order's identifier.
       * @param delta The change in size.
       * @param timestamp The update's timestamp.
       * @return Whether the BboQuote changed.
       */
      bool offset_size(
        const OrderId& id, Quantity delta, boost::posix_time::ptime timestamp);

      /**
       * Sets an order's price.
       * @param id The order's identifier.
       * @param price The new price.
       * @param timestamp The update's timestamp.
       * @return Whether the BboQuote changed.
       */
      bool modify_price(
        const OrderId& id, Money price, boost::posix_time::ptime timestamp);

      /**
       * Removes an order.
       * @param id The order's identifier.
       * @param timestamp The removal's timestamp.
       * @return Whether the BboQuote changed.
       */
      bool remove(const OrderId& id, boost::posix_time::ptime timestamp);

      /**
       * Removes all orders.
       * @param timestamp The removal's timestamp.
       * @return Whether the BboQuote changed.
       */
      bool clear(boost::posix_time::ptime timestamp);

    private:
      using Book = OrderToBookQuoteModel<OrderId>;
      using Updates = typename Book::Updates;
      Book m_bids;
      Book m_asks;
      BboQuote m_bbo;

      bool update(const Updates& bids, const Updates& asks,
        boost::posix_time::ptime timestamp);
      bool update(const Book& book, const Updates& updates);
  };

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBboQuoteModel<O>::OrderToBboQuoteModel()
    : m_bids(Side::BID),
      m_asks(Side::ASK) {}

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  const BboQuote& OrderToBboQuoteModel<O>::get_bbo() const {
    return m_bbo;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::add(const OrderId& id, BookQuote order) {
    auto timestamp = order.m_timestamp;
    auto bids = Updates();
    auto asks = Updates();
    if(order.m_quote.m_side == Side::BID) {
      asks = m_asks.remove(id, timestamp);
      bids = m_bids.add(id, std::move(order));
    } else {
      bids = m_bids.remove(id, timestamp);
      asks = m_asks.add(id, std::move(order));
    }
    return update(bids, asks, timestamp);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::modify_size(
      const OrderId& id, Quantity size, boost::posix_time::ptime timestamp) {
    auto bids = m_bids.modify_size(id, size, timestamp);
    auto asks = m_asks.modify_size(id, size, timestamp);
    return update(bids, asks, timestamp);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::offset_size(
      const OrderId& id, Quantity delta, boost::posix_time::ptime timestamp) {
    auto bids = m_bids.offset_size(id, delta, timestamp);
    auto asks = m_asks.offset_size(id, delta, timestamp);
    return update(bids, asks, timestamp);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::modify_price(
      const OrderId& id, Money price, boost::posix_time::ptime timestamp) {
    auto bids = m_bids.modify_price(id, price, timestamp);
    auto asks = m_asks.modify_price(id, price, timestamp);
    return update(bids, asks, timestamp);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::remove(
      const OrderId& id, boost::posix_time::ptime timestamp) {
    auto bids = m_bids.remove(id, timestamp);
    auto asks = m_asks.remove(id, timestamp);
    return update(bids, asks, timestamp);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::clear(boost::posix_time::ptime timestamp) {
    auto bids = m_bids.clear(timestamp);
    auto asks = m_asks.clear(timestamp);
    return update(bids, asks, timestamp);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::update(const Updates& bids,
      const Updates& asks, boost::posix_time::ptime timestamp) {
    auto is_bid_changed = update(m_bids, bids);
    auto is_ask_changed = update(m_asks, asks);
    if(!is_bid_changed && !is_ask_changed) {
      return false;
    }
    m_bbo.m_timestamp = timestamp;
    return true;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBboQuoteModel<O>::update(
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
