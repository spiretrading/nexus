#ifndef NEXUS_ORDER_FIELDS_HPP
#define NEXUS_ORDER_FIELDS_HPP
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Asset.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"

namespace Nexus {

  /** Contains the fields needed to represent an Order. */
  struct OrderFields {

    /** The account to assign the Order to. */
    Beam::DirectoryEntry m_account;

    /** The ticker the order was submitted for. */
    Ticker m_ticker;

    /** The currency being used. */
    Asset m_currency;

    /** The type of Order. */
    OrderType m_type;

    /** The Side of the Order. */
    Side m_side;

    /** The destination to submit the Order to. */
    Destination m_destination;

    /** The quantity to order. */
    Quantity m_quantity;

    /** The price of the Order. */
    Money m_price;

    /** The Order's time in force. */
    TimeInForce m_time_in_force;

    /** Carries any additional fields to submit. */
    std::vector<Tag> m_additional_fields;

    /** Constructs an OrderFields. */
    OrderFields() noexcept;

    /**
     * Constructs an OrderFields.
     * @param account The account to assign the Order to.
     * @param ticker The ticker the order was submitted for.
     * @param currency The currency being used.
     * @param type The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     * @param time_in_force The Order's time in force.
     * @param additional_fields Carries any additional fields to submit.
     */
    OrderFields(Beam::DirectoryEntry account, Ticker ticker, Asset currency,
      OrderType type, Side side, Destination destination, Quantity quantity,
      Money price, TimeInForce time_in_force,
      std::vector<Tag> additional_fields) noexcept;

    /**
     * Returns <code>true</code> iff one OrderFields comes before another.
     * @param rhs The right hand side of the comparison.
     * @return <code>true</code> iff <code>this</code> comes before <i>rhs</i>.
     */
    bool operator <(const OrderFields& rhs) const;

    bool operator ==(const OrderFields&) const = default;
  };

  /**
   * Returns OrderFields for a LIMIT order with all mandatory fields populated.
   * @param account The account to assign the Order to.
   * @param ticker The ticker the order was submitted for.
   * @param currency The currency being used.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   * @param price The price of the Order.
   */
  inline OrderFields make_limit_order_fields(Beam::DirectoryEntry account,
      Ticker ticker, Asset currency, Side side, Destination destination,
      Quantity quantity, Money price) {
    return OrderFields(std::move(account), std::move(ticker), currency,
      OrderType::LIMIT, side, std::move(destination), quantity, price,
      TimeInForce::Type::DAY, {});
  }

  /**
   * Returns OrderFields for a LIMIT order with all mandatory fields populated.
   * @param ticker The ticker the order was submitted for.
   * @param currency The currency being used.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   * @param price The price of the Order.
   */
  inline OrderFields make_limit_order_fields(Ticker ticker, Asset currency,
      Side side, Destination destination, Quantity quantity, Money price) {
    return make_limit_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      currency, side, std::move(destination), quantity, price);
  }

  /**
   * Returns OrderFields for a LIMIT order with all mandatory fields populated.
   * @param account The account to assign the Order to.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   * @param price The price of the Order.
   */
  inline OrderFields make_limit_order_fields(Beam::DirectoryEntry account,
      Ticker ticker, Side side, Destination destination, Quantity quantity,
      Money price) {
    return make_limit_order_fields(std::move(account), std::move(ticker),
      CurrencyId::NONE, side, std::move(destination), quantity, price);
  }

  /**
   * Returns OrderFields for a LIMIT order with all mandatory fields populated.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   * @param price The price of the Order.
   */
  inline OrderFields make_limit_order_fields(Ticker ticker, Side side,
      Destination destination, Quantity quantity, Money price) {
    return make_limit_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      CurrencyId::NONE, side, std::move(destination), quantity, price);
  }

  /**
   * Returns OrderFields for a LIMIT order with all mandatory fields populated.
   * @param ticker The ticker the Order was submitted for.
   * @param currency The Currency being used.
   * @param side The Side of the Order.
   * @param quantity The quantity to order.
   * @param price The price of the Order.
   */
  inline OrderFields make_limit_order_fields(Ticker ticker, Asset currency,
      Side side, Quantity quantity, Money price) {
    return make_limit_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      currency, side, {}, quantity, price);
  }

  /**
   * Returns OrderFields for a LIMIT order with all mandatory fields populated.
   * @param account The account to assign the Order to.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param quantity The quantity to order.
   * @param price The price of the Order.
   */
  inline OrderFields make_limit_order_fields(Beam::DirectoryEntry account,
      Ticker ticker, Side side, Quantity quantity, Money price) {
    return make_limit_order_fields(std::move(account), std::move(ticker),
      CurrencyId::NONE, side, {}, quantity, price);
  }

  /**
   * Returns OrderFields for a LIMIT order with all mandatory fields
   * populated.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param quantity The quantity to order.
   * @param price The price of the Order.
   */
  inline OrderFields make_limit_order_fields(Ticker ticker, Side side,
      Quantity quantity, Money price) {
    return make_limit_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      CurrencyId::NONE, side, {}, quantity, price);
  }

  /**
   * Returns OrderFields for a MARKET order with all mandatory fields
   * populated.
   * @param account The account to assign the Order to.
   * @param ticker The ticker the Order was submitted for.
   * @param currency The Currency being used.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   */
  inline OrderFields make_market_order_fields(Beam::DirectoryEntry account,
      Ticker ticker, Asset currency, Side side, Destination destination,
      Quantity quantity) {
    return OrderFields(std::move(account), std::move(ticker), currency,
      OrderType::MARKET, side, std::move(destination), quantity, Money::ZERO,
      TimeInForce::Type::DAY, {});
  }

  /**
   * Returns OrderFields for a MARKET order with all mandatory fields
   * populated.
   * @param ticker The ticker the Order was submitted for.
   * @param currency The Currency being used.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   */
  inline OrderFields make_market_order_fields(Ticker ticker, Asset currency,
      Side side, Destination destination, Quantity quantity) {
    return make_market_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      currency, side, std::move(destination), quantity);
  }

  /**
   * Returns OrderFields for a MARKET order with all mandatory fields
   * populated.
   * @param account The account to assign the Order to.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   */
  inline OrderFields make_market_order_fields(Beam::DirectoryEntry account,
      Ticker ticker, Side side, Destination destination, Quantity quantity) {
    return make_market_order_fields(std::move(account), std::move(ticker),
      CurrencyId::NONE, side, std::move(destination), quantity);
  }

  /**
   * Returns OrderFields for a MARKET order with all mandatory fields
   * populated.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param destination The destination to submit the Order to.
   * @param quantity The quantity to order.
   */
  inline OrderFields make_market_order_fields(Ticker ticker, Side side,
      Destination destination, Quantity quantity) {
    return make_market_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      CurrencyId::NONE, side, std::move(destination), quantity);
  }

  /**
   * Returns OrderFields for a MARKET order with all mandatory fields
   * populated.
   * @param ticker The ticker the Order was submitted for.
   * @param currency The Currency being used.
   * @param side The Side of the Order.
   * @param quantity The quantity to order.
   */
  inline OrderFields make_market_order_fields(Ticker ticker, Asset currency,
      Side side, Quantity quantity) {
    return make_market_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      currency, side, {}, quantity);
  }

  /**
   * Returns OrderFields for a MARKET order with all mandatory fields
   * populated.
   * @param account The account to assign the Order to.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param quantity The quantity to order.
   */
  inline OrderFields make_market_order_fields(Beam::DirectoryEntry account,
      Ticker ticker, Side side, Quantity quantity) {
    return make_market_order_fields(std::move(account), std::move(ticker),
      CurrencyId::NONE, side, {}, quantity);
  }

  /**
   * Returns OrderFields for a MARKET order with all mandatory fields
   * populated.
   * @param ticker The ticker the Order was submitted for.
   * @param side The Side of the Order.
   * @param quantity The quantity to order.
   */
  inline OrderFields make_market_order_fields(
      Ticker ticker, Side side, Quantity quantity) {
    return make_market_order_fields(Beam::DirectoryEntry(), std::move(ticker),
      CurrencyId::NONE, side, {}, quantity);
  }

  /**
   * Checks if a Tag with a specified key is part of an OrderFields.
   * @param fields The OrderFields to search.
   * @param key The key of the Tag to find.
   * @return The Tag found with the specified <i>key</i>.
   */
  inline boost::optional<Tag> find_field(const OrderFields& fields, int key) {
    auto i = std::find_if(fields.m_additional_fields.begin(),
      fields.m_additional_fields.end(), [&] (const auto& tag) {
        return tag.get_key() == key;
      });
    if(i == fields.m_additional_fields.end()) {
      return boost::none;
    }
    return *i;
  }

  /**
   * Checks if a specified Tag is part of an OrderFields.
   * @param fields The OrderFields to search.
   * @param tag The Tag to find.
   * @return <code>true</code> iff the specified <i>tag</i> was found.
   */
  inline bool has_field(const OrderFields& fields, const Tag& tag) {
    return std::find(fields.m_additional_fields.begin(),
      fields.m_additional_fields.end(), tag) !=
        fields.m_additional_fields.end();
  }

  inline std::ostream& operator <<(
      std::ostream& out, const OrderFields& value) {
    return out << '(' << value.m_account << ' ' << value.m_ticker << ' ' <<
      value.m_currency << ' ' << value.m_type << ' ' << value.m_side << ' ' <<
      value.m_destination << ' ' << value.m_quantity << ' ' << value.m_price <<
      ' ' << value.m_time_in_force << ' ' <<
      Beam::Stream(value.m_additional_fields) << ')';
  }

  inline OrderFields::OrderFields() noexcept
    : m_type(OrderType::NONE),
      m_side(Side::NONE),
      m_quantity(0),
      m_price(Money::ZERO),
      m_time_in_force(TimeInForce::Type::GTC) {}

  inline OrderFields::OrderFields(Beam::DirectoryEntry account, Ticker ticker,
    Asset currency, OrderType type, Side side, Destination destination,
    Quantity quantity, Money price, TimeInForce time_in_force,
    std::vector<Tag> additional_fields) noexcept
    : m_account(std::move(account)),
      m_ticker(std::move(ticker)),
      m_currency(currency),
      m_type(type),
      m_side(side),
      m_destination(std::move(destination)),
      m_quantity(quantity),
      m_price(price),
      m_time_in_force(time_in_force),
      m_additional_fields(std::move(additional_fields)) {}

  inline bool OrderFields::operator <(const OrderFields& rhs) const {
    return m_type < rhs.m_type || (m_type == rhs.m_type &&
      ((m_side == Side::BID && m_price < rhs.m_price) ||
       (m_side == Side::ASK && rhs.m_price < m_price)));
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::OrderFields> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, Nexus::OrderFields& value, unsigned int version) const {
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("ticker", value.m_ticker);
      shuttle.shuttle("currency", value.m_currency);
      shuttle.shuttle("type", value.m_type);
      shuttle.shuttle("side", value.m_side);
      shuttle.shuttle("destination", value.m_destination);
      shuttle.shuttle("quantity", value.m_quantity);
      shuttle.shuttle("price", value.m_price);
      shuttle.shuttle("time_in_force", value.m_time_in_force);
      shuttle.shuttle("additional_fields", value.m_additional_fields);
    }
  };
}

#endif
