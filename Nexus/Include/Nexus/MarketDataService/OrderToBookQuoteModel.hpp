#ifndef NEXUS_ORDER_TO_BOOK_QUOTE_MODEL_HPP
#define NEXUS_ORDER_TO_BOOK_QUOTE_MODEL_HPP
#include <concepts>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <boost/container/small_vector.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Definitions/BookQuote.hpp"

namespace Nexus {

  /**
   * Aggregates orders into BookQuotes.
   * @tparam O The type used to identify orders.
   */
  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  class OrderToBookQuoteModel {
    public:

      /** The type used to identify orders. */
      using OrderId = O;

      /** The aggregate quotes changed by an operation. */
      using Updates = boost::container::small_vector<BookQuote, 2>;

      OrderToBookQuoteModel() = default;

      /**
       * Adds or replaces an order.
       * @param id The order's identifier.
       * @param order The individual order's quote.
       * @return The updated aggregate quotes.
       */
      Updates add(const OrderId& id, const BookQuote& order);

      /**
       * Sets an order's size, removing it if the size is nonpositive.
       * @param id The order's identifier.
       * @param size The new size.
       * @param timestamp The update's timestamp.
       * @return The updated aggregate quotes, or empty if the order is absent.
       */
      Updates modify_size(
        const OrderId& id, Quantity size, boost::posix_time::ptime timestamp);

      /**
       * Adjusts an order's size.
       * @param id The order's identifier.
       * @param delta The change in size.
       * @param timestamp The update's timestamp.
       * @return The updated aggregate quotes, or empty if the order is absent.
       */
      Updates offset_size(
        const OrderId& id, Quantity delta, boost::posix_time::ptime timestamp);

      /**
       * Sets an order's price.
       * @param id The order's identifier.
       * @param price The new price.
       * @param timestamp The update's timestamp.
       * @return The updated aggregate quotes, or empty if the order is absent.
       */
      Updates modify_price(
        const OrderId& id, Money price, boost::posix_time::ptime timestamp);

      /**
       * Removes an order.
       * @param id The order's identifier.
       * @param timestamp The removal's timestamp.
       * @return The updated aggregate quote, or empty if the order is absent.
       */
      Updates remove(const OrderId& id, boost::posix_time::ptime timestamp);

      /**
       * Removes all orders.
       * @param timestamp The removal's timestamp.
       * @return The quotes clearing every aggregate entry.
       */
      Updates clear(boost::posix_time::ptime timestamp);

    private:
      using Key = std::tuple<Venue, std::string, Side, Money>;
      struct Entry {
        Quantity m_size;
        int m_primary_count = 0;
      };
      std::unordered_map<OrderId, BookQuote> m_orders;
      std::unordered_map<Key, Entry, boost::hash<Key>> m_quotes;

      BookQuote update(const BookQuote& order, Quantity delta,
        int primary_delta);
  };

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Updates OrderToBookQuoteModel<O>::add(
      const OrderId& id, const BookQuote& order) {
    if(order.m_quote.m_size <= 0) {
      return remove(id, order.m_timestamp);
    }
    auto updates = Updates();
    auto i = m_orders.find(id);
    if(i != m_orders.end()) {
      auto& previous = i->second;
      if(previous.m_venue == order.m_venue && previous.m_mpid == order.m_mpid &&
          previous.m_quote.m_side == order.m_quote.m_side &&
          previous.m_quote.m_price == order.m_quote.m_price) {
        auto delta = order.m_quote.m_size - previous.m_quote.m_size;
        auto primary_delta =
          int(order.m_is_primary_mpid) - int(previous.m_is_primary_mpid);
        if(delta == 0 && primary_delta == 0) {
          return updates;
        }
        updates.push_back(update(order, delta, primary_delta));
        previous = order;
        return updates;
      }
      auto removal = previous;
      removal.m_timestamp = order.m_timestamp;
      updates.push_back(update(
        removal, -removal.m_quote.m_size, -int(removal.m_is_primary_mpid)));
      previous = order;
    } else {
      m_orders.emplace(id, order);
    }
    updates.push_back(
      update(order, order.m_quote.m_size, int(order.m_is_primary_mpid)));
    return updates;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Updates OrderToBookQuoteModel<O>::modify_size(
      const OrderId& id, Quantity size, boost::posix_time::ptime timestamp) {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second;
    order.m_quote.m_size = size;
    order.m_timestamp = timestamp;
    return add(id, order);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Updates OrderToBookQuoteModel<O>::offset_size(
      const OrderId& id, Quantity delta, boost::posix_time::ptime timestamp) {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second;
    order.m_quote.m_size += delta;
    order.m_timestamp = timestamp;
    return add(id, order);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Updates OrderToBookQuoteModel<O>::modify_price(
      const OrderId& id, Money price, boost::posix_time::ptime timestamp) {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second;
    order.m_quote.m_price = price;
    order.m_timestamp = timestamp;
    return add(id, order);
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Updates OrderToBookQuoteModel<O>::remove(
      const OrderId& id, boost::posix_time::ptime timestamp) {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second;
    order.m_timestamp = timestamp;
    auto updates = Updates();
    updates.push_back(
      update(order, -order.m_quote.m_size, -int(order.m_is_primary_mpid)));
    m_orders.erase(i);
    return updates;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Updates OrderToBookQuoteModel<O>::clear(
      boost::posix_time::ptime timestamp) {
    auto updates = Updates();
    updates.reserve(m_quotes.size());
    for(auto& [key, entry] : m_quotes) {
      auto& [venue, mpid, side, price] = key;
      updates.emplace_back(mpid, entry.m_primary_count != 0, venue,
        Quote(price, 0, side), timestamp);
    }
    m_orders.clear();
    m_quotes.clear();
    return updates;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  BookQuote OrderToBookQuoteModel<O>::update(
      const BookQuote& order, Quantity delta, int primary_delta) {
    auto key = Key(
      order.m_venue, order.m_mpid, order.m_quote.m_side, order.m_quote.m_price);
    auto i = m_quotes.try_emplace(std::move(key)).first;
    auto& entry = i->second;
    entry.m_size += delta;
    entry.m_primary_count += primary_delta;
    auto quote = order;
    quote.m_quote.m_size = entry.m_size;
    quote.m_is_primary_mpid = entry.m_primary_count != 0;
    if(entry.m_size == 0) {
      m_quotes.erase(i);
    }
    return quote;
  }
}

#endif
