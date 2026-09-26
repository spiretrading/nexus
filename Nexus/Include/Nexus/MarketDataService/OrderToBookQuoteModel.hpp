#ifndef NEXUS_ORDER_TO_BOOK_QUOTE_MODEL_HPP
#define NEXUS_ORDER_TO_BOOK_QUOTE_MODEL_HPP
#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/container/small_vector.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/BookQuote.hpp"

namespace Nexus {
namespace Details {
  template<typename O>
  struct OrderQuoteEntry {
    O m_order;
    BookQuote m_contribution;
  };
}

  /** Concept satisfied by adapters exposing order attributes. */
  template<typename A>
  concept IsOrderAdapter = std::copyable<A> &&
    requires(const A& adapter) {
      typename A::OrderId;
      typename A::Order;
      requires std::copy_constructible<typename A::OrderId>;
      requires std::equality_comparable<typename A::OrderId>;
      requires std::invocable<
        std::hash<typename A::OrderId>, const typename A::OrderId&>;
      requires std::copyable<typename A::Order>;
      { adapter.get_price(std::declval<const typename A::Order&>()) } ->
        std::same_as<Money>;
      { adapter.get_quantity(std::declval<const typename A::Order&>()) } ->
        std::same_as<Quantity>;
      { adapter.get_mpid(std::declval<const typename A::Order&>()) } ->
        std::convertible_to<std::string>;
      { adapter.get_venue(std::declval<const typename A::Order&>()) } ->
        std::same_as<Venue>;
      { adapter.is_primary_mpid(std::declval<const typename A::Order&>()) } ->
        std::same_as<bool>;
    };

  /**
   * Exposes the order attributes of individual BookQuotes.
   * @tparam I The type used to identify orders.
   */
  template<std::copy_constructible I> requires
    std::equality_comparable<I> && std::invocable<std::hash<I>, const I&>
  struct BookQuoteOrderAdapter {

    /** The type used to identify orders. */
    using OrderId = I;

    /** The individual order. */
    using Order = BookQuote;

    /** Returns an order's price. */
    Money get_price(const Order& order) const;

    /** Returns an order's quantity. */
    Quantity get_quantity(const Order& order) const;

    /** Returns an order's MPID. */
    const std::string& get_mpid(const Order& order) const;

    /** Returns an order's venue. */
    Venue get_venue(const Order& order) const;

    /** Returns whether an order's MPID is primary. */
    bool is_primary_mpid(const Order& order) const;
  };

  /**
   * Maintains one side of an order book by aggregating order contributions.
   * @tparam A The adapter exposing order attributes.
   */
  template<IsOrderAdapter A>
  class OrderToBookQuoteModel {
    public:

      /** Exposes the attributes of stored orders. */
      using Adapter = A;

      /** The type used to identify orders. */
      using OrderId = typename Adapter::OrderId;

      /** The original order record. */
      using Order = typename Adapter::Order;

      /** A read-only view of order identifiers and original records. */
      using Orders = std::ranges::transform_view<
        std::ranges::ref_view<const std::unordered_map<
          OrderId, Details::OrderQuoteEntry<Order>>>,
        decltype([] (const auto& entry) {
          return std::pair<const OrderId&, const Order&>(
            entry.first, entry.second.m_order);
        })>;

      /** The aggregate quotes changed by an operation. */
      using Updates = boost::container::small_vector<BookQuote, 2>;

      /** Iterates over aggregate quotes from best to worst. */
      using Iterator = decltype((std::declval<
        const std::vector<std::pair<BookQuote, int>>&>() |
          std::views::reverse | std::views::keys).begin());

      /** Constructs an empty book using a default adapter. */
      explicit OrderToBookQuoteModel(Side side) requires
        std::default_initializable<Adapter>;

      /**
       * Constructs an empty book.
       * @param side The side to maintain.
       * @param adapter Provides the attributes of stored orders.
       */
      OrderToBookQuoteModel(Side side, Adapter adapter);

      /** Returns the side maintained by this model. */
      Side get_side() const;

      /** Returns the adapter exposing order attributes. */
      const Adapter& get_adapter() const;

      /** Replaces the adapter and refreshes all order contributions. */
      Updates set_adapter(Adapter adapter, boost::posix_time::ptime timestamp);

      /** Returns an original order, or none if it is absent. */
      boost::optional<const Order&> find_order(const OrderId& id) const;

      /** Returns a view of original orders, including unpublished orders. */
      Orders get_orders() const;

      /** Returns the aggregate quote at a best-to-worst index. */
      const BookQuote& operator [](std::size_t index) const;
      Iterator begin() const;
      Iterator end() const;
      std::size_t size() const;
      bool empty() const;

      /**
       * Adds or replaces an original order and updates its contribution.
       * Orders with nonpositive contributions remain stored until removed.
       * @param id The order's identifier.
       * @param order The original order record.
       * @param timestamp The update's timestamp.
       * @return The updated aggregate quotes.
       */
      Updates add(
        const OrderId& id, Order order, boost::posix_time::ptime timestamp);

      /** Adds a BookQuote, removing the order if its size is nonpositive. */
      Updates add(const OrderId& id, BookQuote order) requires
        std::same_as<Order, BookQuote>;

      /**
       * Modifies an existing order and updates its contribution.
       * @param id The order's identifier.
       * @param f Modifies the original order record.
       * @param timestamp The update's timestamp.
       * @return The updated aggregate quotes, or empty if the order is absent.
       */
      template<std::invocable<typename A::Order&> F>
      Updates update(const OrderId& id, F f, boost::posix_time::ptime timestamp);

      /** Modifies matching orders and updates their contributions. */
      template<std::predicate<const typename A::Order&> P,
        std::invocable<typename A::Order&> F>
      Updates update_if(P predicate, F f, boost::posix_time::ptime timestamp);

      /** Recalculates all contributions from their original orders. */
      Updates refresh(boost::posix_time::ptime timestamp);

      /** Sets a BookQuote order's size, removing nonpositive orders. */
      Updates modify_size(const OrderId& id, Quantity size,
        boost::posix_time::ptime timestamp) requires
          std::same_as<Order, BookQuote>;

      /** Adjusts a BookQuote order's size, removing nonpositive orders. */
      Updates offset_size(const OrderId& id, Quantity delta,
        boost::posix_time::ptime timestamp) requires
          std::same_as<Order, BookQuote>;

      /** Sets a BookQuote order's price. */
      Updates modify_price(const OrderId& id, Money price,
        boost::posix_time::ptime timestamp) requires
          std::same_as<Order, BookQuote>;

      /** Removes an original order and its quote contribution. */
      Updates remove(const OrderId& id, boost::posix_time::ptime timestamp);

      /** Removes all orders and returns quotes clearing the aggregate book. */
      Updates clear(boost::posix_time::ptime timestamp);

    private:
      using Entry = Details::OrderQuoteEntry<Order>;
      using OrderMap = std::unordered_map<OrderId, Entry>;
      Side m_side;
      Adapter m_adapter;
      OrderMap m_orders;
      std::vector<std::pair<BookQuote, int>> m_quotes;

      BookQuote make_quote(
        const Order& order, boost::posix_time::ptime timestamp) const;
      Updates replace(
        Entry& entry, Order order, boost::posix_time::ptime timestamp);
      BookQuote update_quote(
        const BookQuote& order, Quantity delta, int primary_delta);
  };

  template<typename A>
  OrderToBookQuoteModel(Side, A) -> OrderToBookQuoteModel<A>;

  template<std::copy_constructible I> requires
    std::equality_comparable<I> && std::invocable<std::hash<I>, const I&>
  Money BookQuoteOrderAdapter<I>::get_price(const Order& order) const {
    return order.m_quote.m_price;
  }

  template<std::copy_constructible I> requires
    std::equality_comparable<I> && std::invocable<std::hash<I>, const I&>
  Quantity BookQuoteOrderAdapter<I>::get_quantity(const Order& order) const {
    return order.m_quote.m_size;
  }

  template<std::copy_constructible I> requires
    std::equality_comparable<I> && std::invocable<std::hash<I>, const I&>
  const std::string& BookQuoteOrderAdapter<I>::get_mpid(
      const Order& order) const {
    return order.m_mpid;
  }

  template<std::copy_constructible I> requires
    std::equality_comparable<I> && std::invocable<std::hash<I>, const I&>
  Venue BookQuoteOrderAdapter<I>::get_venue(const Order& order) const {
    return order.m_venue;
  }

  template<std::copy_constructible I> requires
    std::equality_comparable<I> && std::invocable<std::hash<I>, const I&>
  bool BookQuoteOrderAdapter<I>::is_primary_mpid(const Order& order) const {
    return order.m_is_primary_mpid;
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::OrderToBookQuoteModel(Side side) requires
    std::default_initializable<Adapter>
    : OrderToBookQuoteModel(side, Adapter()) {}

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::OrderToBookQuoteModel(Side side, Adapter adapter)
    : m_side(side),
      m_adapter(std::move(adapter)) {}

  template<IsOrderAdapter A>
  Side OrderToBookQuoteModel<A>::get_side() const {
    return m_side;
  }

  template<IsOrderAdapter A>
  const OrderToBookQuoteModel<A>::Adapter&
      OrderToBookQuoteModel<A>::get_adapter() const {
    return m_adapter;
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::set_adapter(
      Adapter adapter, boost::posix_time::ptime timestamp) {
    m_adapter = std::move(adapter);
    return refresh(timestamp);
  }

  template<IsOrderAdapter A>
  boost::optional<const typename OrderToBookQuoteModel<A>::Order&>
      OrderToBookQuoteModel<A>::find_order(const OrderId& id) const {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return boost::none;
    }
    return i->second.m_order;
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Orders
      OrderToBookQuoteModel<A>::get_orders() const {
    return Orders(m_orders, {});
  }

  template<IsOrderAdapter A>
  const BookQuote& OrderToBookQuoteModel<A>::operator [](
      std::size_t index) const {
    return m_quotes[m_quotes.size() - index - 1].first;
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Iterator OrderToBookQuoteModel<A>::begin() const {
    return (m_quotes | std::views::reverse | std::views::keys).begin();
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Iterator OrderToBookQuoteModel<A>::end() const {
    return (m_quotes | std::views::reverse | std::views::keys).end();
  }

  template<IsOrderAdapter A>
  std::size_t OrderToBookQuoteModel<A>::size() const {
    return m_quotes.size();
  }

  template<IsOrderAdapter A>
  bool OrderToBookQuoteModel<A>::empty() const {
    return m_quotes.empty();
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::add(
      const OrderId& id, Order order, boost::posix_time::ptime timestamp) {
    auto i = m_orders.find(id);
    if(i != m_orders.end()) {
      return replace(i->second, std::move(order), timestamp);
    }
    auto contribution = make_quote(order, timestamp);
    auto updates = Updates();
    if(contribution.m_quote.m_size > 0) {
      updates.push_back(update_quote(contribution, contribution.m_quote.m_size,
        int(contribution.m_is_primary_mpid)));
    }
    m_orders.emplace(id, Entry(std::move(order), std::move(contribution)));
    return updates;
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::add(
      const OrderId& id, BookQuote order) requires
        std::same_as<Order, BookQuote> {
    if(order.m_quote.m_size <= 0) {
      return remove(id, order.m_timestamp);
    }
    auto timestamp = order.m_timestamp;
    return add(id, std::move(order), timestamp);
  }

  template<IsOrderAdapter A>
  template<std::invocable<typename A::Order&> F>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::update(
      const OrderId& id, F f, boost::posix_time::ptime timestamp) {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second.m_order;
    std::invoke(f, order);
    return replace(i->second, std::move(order), timestamp);
  }

  template<IsOrderAdapter A>
  template<std::predicate<const typename A::Order&> P,
    std::invocable<typename A::Order&> F>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::update_if(
      P predicate, F f, boost::posix_time::ptime timestamp) {
    auto updates = Updates();
    for(auto& [id, entry] : m_orders) {
      if(!std::invoke(predicate, std::as_const(entry.m_order))) {
        continue;
      }
      auto order = entry.m_order;
      std::invoke(f, order);
      auto quotes = replace(entry, std::move(order), timestamp);
      updates.insert(updates.end(), std::make_move_iterator(quotes.begin()),
        std::make_move_iterator(quotes.end()));
    }
    return updates;
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::refresh(
      boost::posix_time::ptime timestamp) {
    return update_if([] (const auto&) { return true; }, [] (auto&) {},
      timestamp);
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::modify_size(
      const OrderId& id, Quantity size,
      boost::posix_time::ptime timestamp) requires
        std::same_as<Order, BookQuote> {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second.m_order;
    order.m_quote.m_size = size;
    order.m_timestamp = timestamp;
    return add(id, std::move(order));
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::offset_size(
      const OrderId& id, Quantity delta,
      boost::posix_time::ptime timestamp) requires
        std::same_as<Order, BookQuote> {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second.m_order;
    order.m_quote.m_size += delta;
    order.m_timestamp = timestamp;
    return add(id, std::move(order));
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::modify_price(
      const OrderId& id, Money price,
      boost::posix_time::ptime timestamp) requires
        std::same_as<Order, BookQuote> {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto order = i->second.m_order;
    order.m_quote.m_price = price;
    order.m_timestamp = timestamp;
    return add(id, std::move(order));
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::remove(
      const OrderId& id, boost::posix_time::ptime timestamp) {
    auto i = m_orders.find(id);
    if(i == m_orders.end()) {
      return {};
    }
    auto contribution = i->second.m_contribution;
    contribution.m_timestamp = timestamp;
    auto updates = Updates();
    if(contribution.m_quote.m_size > 0) {
      updates.push_back(update_quote(contribution,
        -contribution.m_quote.m_size, -int(contribution.m_is_primary_mpid)));
    }
    m_orders.erase(i);
    return updates;
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::clear(
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

  template<IsOrderAdapter A>
  BookQuote OrderToBookQuoteModel<A>::make_quote(
      const Order& order, boost::posix_time::ptime timestamp) const {
    return BookQuote(
      m_adapter.get_mpid(order), m_adapter.is_primary_mpid(order),
      m_adapter.get_venue(order),
      Quote(m_adapter.get_price(order), m_adapter.get_quantity(order), m_side),
      timestamp);
  }

  template<IsOrderAdapter A>
  OrderToBookQuoteModel<A>::Updates OrderToBookQuoteModel<A>::replace(
      Entry& entry, Order order, boost::posix_time::ptime timestamp) {
    auto contribution = make_quote(order, timestamp);
    auto& previous = entry.m_contribution;
    auto updates = Updates();
    if(previous.m_quote.m_size > 0 && contribution.m_quote.m_size > 0 &&
        previous.m_venue == contribution.m_venue &&
        previous.m_mpid == contribution.m_mpid &&
        previous.m_quote.m_price == contribution.m_quote.m_price) {
      auto delta = contribution.m_quote.m_size - previous.m_quote.m_size;
      auto primary_delta = int(contribution.m_is_primary_mpid) -
        int(previous.m_is_primary_mpid);
      if(delta != 0 || primary_delta != 0) {
        updates.push_back(update_quote(contribution, delta, primary_delta));
      }
    } else {
      if(previous.m_quote.m_size > 0) {
        auto removal = previous;
        removal.m_timestamp = timestamp;
        updates.push_back(update_quote(
          removal, -removal.m_quote.m_size, -int(removal.m_is_primary_mpid)));
      }
      if(contribution.m_quote.m_size > 0) {
        updates.push_back(update_quote(contribution,
          contribution.m_quote.m_size, int(contribution.m_is_primary_mpid)));
      }
    }
    entry.m_order = std::move(order);
    previous = std::move(contribution);
    return updates;
  }

  template<IsOrderAdapter A>
  BookQuote OrderToBookQuoteModel<A>::update_quote(
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
