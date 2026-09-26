#ifndef NEXUS_ORDER_TO_BOOK_QUOTE_MODEL_HPP
#define NEXUS_ORDER_TO_BOOK_QUOTE_MODEL_HPP
#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/container/small_vector.hpp>
#include "Nexus/Definitions/BookQuote.hpp"

namespace Nexus {

  /**
   * Maintains one side of an order book by aggregating orders into BookQuotes.
   * @tparam O The type used to identify orders.
   */
  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  class OrderToBookQuoteModel {
    public:

      /** The type used to identify orders. */
      using OrderId = O;

      /** The individual orders indexed by identifier. */
      using Orders = std::unordered_map<OrderId, BookQuote>;

      /** The aggregate quotes changed by an operation. */
      using Updates = boost::container::small_vector<BookQuote, 2>;

      /** Iterates over aggregate quotes from best to worst. */
      using Iterator = decltype((std::declval<
        const std::vector<std::pair<BookQuote, int>>&>() |
          std::views::reverse | std::views::keys).begin());

      /**
       * Constructs an empty book for a side.
       * @param side The side to maintain.
       */
      explicit OrderToBookQuoteModel(Side side);

      /** Returns the side maintained by this model. */
      Side get_side() const;

      /**
       * Finds an individual order.
       * @param id The order's identifier.
       * @return The order, or nullptr if it is absent.
       */
      const BookQuote* find_order(const OrderId& id) const;

      /** Returns the individual orders indexed by identifier. */
      const Orders& get_orders() const;

      /** Returns the aggregate quote at a best-to-worst index. */
      const BookQuote& operator [](std::size_t index) const;
      Iterator begin() const;
      Iterator end() const;
      std::size_t size() const;
      bool empty() const;

      /**
       * Adds or replaces an order.
       * @param id The order's identifier.
       * @param order The individual order's quote, on this model's side.
       * @return The updated aggregate quotes.
       */
      Updates add(const OrderId& id, BookQuote order);

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
      Side m_side;
      Orders m_orders;
      std::vector<std::pair<BookQuote, int>> m_quotes;

      BookQuote update(
        const BookQuote& order, Quantity delta, int primary_delta);
  };

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::OrderToBookQuoteModel(Side side)
    : m_side(side) {}

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  Side OrderToBookQuoteModel<O>::get_side() const {
    return m_side;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  const BookQuote* OrderToBookQuoteModel<O>::find_order(
      const OrderId& id) const {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return nullptr;
    }
    return &i->second;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  const OrderToBookQuoteModel<O>::Orders&
      OrderToBookQuoteModel<O>::get_orders() const {
    return m_orders;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  const BookQuote& OrderToBookQuoteModel<O>::operator [](
      std::size_t index) const {
    return m_quotes[m_quotes.size() - index - 1].first;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Iterator OrderToBookQuoteModel<O>::begin() const {
    return (m_quotes | std::views::reverse | std::views::keys).begin();
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Iterator OrderToBookQuoteModel<O>::end() const {
    return (m_quotes | std::views::reverse | std::views::keys).end();
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  std::size_t OrderToBookQuoteModel<O>::size() const {
    return m_quotes.size();
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  bool OrderToBookQuoteModel<O>::empty() const {
    return m_quotes.empty();
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  OrderToBookQuoteModel<O>::Updates OrderToBookQuoteModel<O>::add(
      const OrderId& id, BookQuote order) {
    if(order.m_quote.m_size <= 0) {
      return remove(id, order.m_timestamp);
    }
    auto updates = Updates();
    auto i = m_orders.find(id);
    if(i != m_orders.end()) {
      auto& previous = i->second;
      if(previous.m_venue == order.m_venue && previous.m_mpid == order.m_mpid &&
          previous.m_quote.m_price == order.m_quote.m_price) {
        auto delta = order.m_quote.m_size - previous.m_quote.m_size;
        auto primary_delta =
          int(order.m_is_primary_mpid) - int(previous.m_is_primary_mpid);
        if(delta == 0 && primary_delta == 0) {
          return updates;
        }
        updates.push_back(update(order, delta, primary_delta));
        previous = std::move(order);
        return updates;
      }
      auto removal = previous;
      removal.m_timestamp = order.m_timestamp;
      updates.push_back(update(
        removal, -removal.m_quote.m_size, -int(removal.m_is_primary_mpid)));
    }
    updates.push_back(
      update(order, order.m_quote.m_size, int(order.m_is_primary_mpid)));
    if(i != m_orders.end()) {
      i->second = std::move(order);
    } else {
      m_orders.emplace(id, std::move(order));
    }
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
    return add(id, std::move(order));
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
    return add(id, std::move(order));
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
    return add(id, std::move(order));
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
    for(auto& quote : *this) {
      auto removal = quote;
      removal.m_quote.m_size = 0;
      removal.m_timestamp = timestamp;
      updates.push_back(std::move(removal));
    }
    m_orders.clear();
    m_quotes.clear();
    return updates;
  }

  template<std::copy_constructible O> requires
    std::equality_comparable<O> && std::invocable<std::hash<O>, const O&>
  BookQuote OrderToBookQuoteModel<O>::update(
      const BookQuote& order, Quantity delta, int primary_delta) {
    auto i = std::ranges::lower_bound(m_quotes, order,
      [&] (const auto& lhs, const auto& rhs) {
        if(lhs.m_quote.m_price != rhs.m_quote.m_price) {
          return offer_comparator(
            m_side, lhs.m_quote.m_price, rhs.m_quote.m_price) > 0;
        }
        return std::tie(lhs.m_venue, lhs.m_mpid) >
          std::tie(rhs.m_venue, rhs.m_mpid);
      }, &std::pair<BookQuote, int>::first);
    if(i == m_quotes.end() || i->first.m_venue != order.m_venue ||
        i->first.m_mpid != order.m_mpid ||
        i->first.m_quote.m_price != order.m_quote.m_price) {
      auto quote = order;
      quote.m_quote.m_size = 0;
      i = m_quotes.emplace(i, std::move(quote), 0);
    }
    auto& [entry, primary_count] = *i;
    entry.m_quote.m_size += delta;
    primary_count += primary_delta;
    entry.m_is_primary_mpid = primary_count != 0;
    entry.m_timestamp = order.m_timestamp;
    auto quote = entry;
    if(entry.m_quote.m_size == 0) {
      m_quotes.erase(i);
    }
    return quote;
  }
}

#endif
