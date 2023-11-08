#ifndef NEXUS_ORDER_FIELDS_HPP
#define NEXUS_ORDER_FIELDS_HPP
#include <ostream>
#include <string>
#include <tuple>
#include <vector>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Contains the fields needed to represent an Order. */
  struct OrderFields {

    /** The account to assign the Order to. */
    Beam::ServiceLocator::DirectoryEntry m_account;

    /** The Security the Order was submitted for. */
    Security m_security;

    /** The Currency being used. */
    CurrencyId m_currency;

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

    /** The Order's TimeInForce. */
    TimeInForce m_timeInForce;

    /** Carries any additional fields to submit. */
    std::vector<Tag> m_additionalFields;

    /**
     * Returns OrderFields for a LIMIT order with all mandatory fields
     * populated.
     * @param account The account to assign the Order to.
     * @param security The Security the Order was submitted for.
     * @param currency The Currency being used.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const Destination& destination, Quantity quantity, Money price);

    /**
     * Returns OrderFields for a LIMIT order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param currency The Currency being used.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeLimitOrder(const Security& security,
      CurrencyId currency, Side side, const Destination& destination,
      Quantity quantity, Money price);

    /**
     * Returns OrderFields for a LIMIT order with all mandatory fields
     * populated.
     * @param account The account to assign the Order to.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, const Destination& destination,
      Quantity quantity, Money price);

    /**
     * Returns OrderFields for a LIMIT order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeLimitOrder(const Security& security, Side side,
      const Destination& destination, Quantity quantity, Money price);

    /**
     * Returns OrderFields for a LIMIT order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param currency The Currency being used.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeLimitOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity, Money price);

    /**
     * Returns OrderFields for a LIMIT order with all mandatory fields
     * populated.
     * @param account The account to assign the Order to.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity, Money price);

    /**
     * Returns OrderFields for a LIMIT order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeLimitOrder(const Security& security, Side side,
      Quantity quantity, Money price);

    /**
     * Returns OrderFields for a MARKET order with all mandatory fields
     * populated.
     * @param account The account to assign the Order to.
     * @param security The Security the Order was submitted for.
     * @param currency The Currency being used.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const Destination& destination, Quantity quantity);

    /**
     * Returns OrderFields for a MARKET order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param currency The Currency being used.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeMarketOrder(const Security& security,
      CurrencyId currency, Side side, const Destination& destination,
      Quantity quantity);

    /**
     * Returns OrderFields for a MARKET order with all mandatory fields
     * populated.
     * @param account The account to assign the Order to.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, const Destination& destination,
      Quantity quantity);

    /**
     * Returns OrderFields for a MARKET order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param destination The destination to submit the Order to.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeMarketOrder(const Security& security, Side side,
      const Destination& destination, Quantity quantity);

    /**
     * Returns OrderFields for a MARKET order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param currency The Currency being used.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeMarketOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity);

    /**
     * Returns OrderFields for a MARKET order with all mandatory fields
     * populated.
     * @param account The account to assign the Order to.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity);

    /**
     * Returns OrderFields for a MARKET order with all mandatory fields
     * populated.
     * @param security The Security the Order was submitted for.
     * @param orderType The type of Order.
     * @param side The Side of the Order.
     * @param quantity The quantity to order.
     * @param price The price of the Order.
     */
    static OrderFields MakeMarketOrder(const Security& security, Side side,
      Quantity quantity);

    /** Constructs an OrderFields. */
    OrderFields();

    /**
     * Returns <code>true</code> iff one OrderFields comes before another.
     * @param rhs The right hand side of the comparison.
     * @return <code>true</code> iff <code>this</code> comes before <i>rhs</i>.
     */
    bool operator <(const OrderFields& rhs) const;

    bool operator ==(const OrderFields& rhs) const = default;
  };

  /**
   * Checks if a Tag with a specified key is part of an OrderFields.
   * @param fields The OrderFields to search.
   * @param key The key of the Tag to find.
   * @return The Tag found with the specified <i>key</i>.
   */
  inline boost::optional<Tag> FindField(const OrderFields& fields, int key) {
    for(auto& tag : fields.m_additionalFields) {
      if(tag.GetKey() == key) {
        return tag;
      }
    }
    return boost::none;
  }

  /**
   * Checks if a specified Tag is part of an OrderFields.
   * @param fields The OrderFields to search.
   * @param tag The Tag to find.
   * @return <code>true</code> iff the specified <i>tag</i> was found.
   */
  inline bool HasField(const OrderFields& fields, const Tag& tag) {
    for(auto& t : fields.m_additionalFields) {
      if(t == tag) {
        return true;
      }
    }
    return false;
  }

  inline std::ostream& operator <<(std::ostream& out,
      const OrderFields& value) {
    return out << "(" << value.m_account << " " << value.m_security << " " <<
      value.m_currency << " " << value.m_type << " " << value.m_side << " " <<
      value.m_destination << " " << value.m_quantity << " " << value.m_price <<
      " " << value.m_timeInForce << " " <<
      Beam::Stream(value.m_additionalFields) << ")";
  }

  inline OrderFields OrderFields::MakeLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const Destination& destination, Quantity quantity, Money price) {
    auto fields = OrderFields();
    fields.m_account = account;
    fields.m_security = security;
    fields.m_currency = currency;
    fields.m_type = OrderType::LIMIT;
    fields.m_side = side;
    fields.m_destination = destination;
    fields.m_quantity = quantity;
    fields.m_price = price;
    fields.m_timeInForce = TimeInForce(TimeInForce::Type::DAY);
    return fields;
  }

  inline OrderFields OrderFields::MakeLimitOrder(const Security& security,
      CurrencyId currency, Side side, const Destination& destination,
      Quantity quantity, Money price) {
    return MakeLimitOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      currency, side, destination, quantity, price);
  }

  inline OrderFields OrderFields::MakeLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, const Destination& destination,
      Quantity quantity, Money price) {
    return MakeLimitOrder(account, security, CurrencyId::NONE, side,
      destination, quantity, price);
  }

  inline OrderFields OrderFields::MakeLimitOrder(const Security& security,
      Side side, const Destination& destination, Quantity quantity,
      Money price) {
    return MakeLimitOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      CurrencyId::NONE, side, destination, quantity, price);
  }

  inline OrderFields OrderFields::MakeLimitOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity, Money price) {
    return MakeLimitOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      currency, side, {}, quantity, price);
  }

  inline OrderFields OrderFields::MakeLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity, Money price) {
    return MakeLimitOrder(account, security, CurrencyId::NONE, side, {},
      quantity, price);
  }

  inline OrderFields OrderFields::MakeLimitOrder(const Security& security,
      Side side, Quantity quantity, Money price) {
    return MakeLimitOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      CurrencyId::NONE, side, {}, quantity, price);
  }

  inline OrderFields OrderFields::MakeMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const Destination& destination, Quantity quantity) {
    auto fields = OrderFields();
    fields.m_account = account;
    fields.m_security = security;
    fields.m_currency = currency;
    fields.m_type = OrderType::MARKET;
    fields.m_side = side;
    fields.m_destination = destination;
    fields.m_quantity = quantity;
    fields.m_timeInForce = TimeInForce(TimeInForce::Type::DAY);
    return fields;
  }

  inline OrderFields OrderFields::MakeMarketOrder(const Security& security,
      CurrencyId currency, Side side, const Destination& destination,
      Quantity quantity) {
    return MakeMarketOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      currency, side, destination, quantity);
  }

  inline OrderFields OrderFields::MakeMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, const Destination& destination,
      Quantity quantity) {
    return MakeMarketOrder(account, security, CurrencyId::NONE, side,
      destination, quantity);
  }

  inline OrderFields OrderFields::MakeMarketOrder(const Security& security,
      Side side, const Destination& destination, Quantity quantity) {
    return MakeMarketOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      CurrencyId::NONE, side, destination, quantity);
  }

  inline OrderFields OrderFields::MakeMarketOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity) {
    return MakeMarketOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      currency, side, {}, quantity);
  }

  inline OrderFields OrderFields::MakeMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity) {
    return MakeMarketOrder(account, security, CurrencyId::NONE, side, {},
      quantity);
  }

  inline OrderFields OrderFields::MakeMarketOrder(const Security& security,
      Side side, Quantity quantity) {
    return MakeMarketOrder(Beam::ServiceLocator::DirectoryEntry(), security,
      CurrencyId::NONE, side, {}, quantity);
  }

  inline OrderFields::OrderFields()
    : m_type(OrderType::NONE),
      m_side(Side::NONE),
      m_quantity(0),
      m_price(Money::ZERO),
      m_timeInForce(TimeInForce::Type::GTC) {}

  inline bool OrderFields::operator <(const OrderFields& rhs) const {
    return m_type < rhs.m_type || (m_type == rhs.m_type &&
      ((m_side == Side::BID && m_price < rhs.m_price) ||
       (m_side == Side::ASK && rhs.m_price < m_price)));
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::OrderExecutionService::OrderFields> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::OrderExecutionService::OrderFields& value,
        unsigned int version) {
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("security", value.m_security);
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("side", value.m_side);
      shuttle.Shuttle("destination", value.m_destination);
      shuttle.Shuttle("quantity", value.m_quantity);
      shuttle.Shuttle("price", value.m_price);
      shuttle.Shuttle("time_in_force", value.m_timeInForce);
      shuttle.Shuttle("additional_fields", value.m_additionalFields);
    }
  };
}

#endif
