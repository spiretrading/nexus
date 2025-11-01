#ifndef NEXUS_QUERY_STANDARD_DATA_TYPES_HPP
#define NEXUS_QUERY_STANDARD_DATA_TYPES_HPP
#include <boost/mp11/list.hpp>
#include <boost/variant/variant.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"

namespace Nexus {
  using QueryVariant = boost::variant<bool, char, int, double, std::uint64_t,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration,
    Quantity, Money, Security, SecurityInfo, OrderImbalance, BboQuote,
    BookQuote, TimeAndSale, OrderFields, OrderInfo>;
  using SequencedQueryVariant = Beam::SequencedValue<QueryVariant>;

  struct QueryTypes {
    using ExtendedNativeTypes = boost::mp11::mp_list<Quantity, Money, Security,
      SecurityInfo, OrderImbalance, BboQuote, BookQuote, TimeAndSale,
      OrderFields, OrderInfo>;
    using NativeTypes = boost::mp11::mp_append<Beam::QueryTypes::NativeTypes,
      ExtendedNativeTypes>;
    using ExtendedValueTypes = boost::mp11::mp_list<Quantity, Money>;
    using ValueTypes = boost::mp11::mp_append<Beam::QueryTypes::ValueTypes,
      ExtendedValueTypes>;
    using ExtendedComparableTypes = boost::mp11::mp_list<Quantity, Money>;
    using ComparableTypes = boost::mp11::mp_append<
      Beam::QueryTypes::ComparableTypes, ExtendedComparableTypes>;
  };

  /**
   * Provides member accessors for an Expression evaluating to a TimeAndSale.
   */
  class TimeAndSaleAccessor {
    public:

      /**
       * Constructs a TimeAndSaleAccessor.
       * @param expression The expression whose members are to be accessed.
       */
      explicit TimeAndSaleAccessor(Beam::Expression expression) noexcept;

      /** Returns an accessor for the timestamp member. */
      Beam::MemberAccessExpression get_timestamp() const;

      /** Returns an accessor for the price member. */
      Beam::MemberAccessExpression get_price() const;

      /** Returns an accessor for the size member. */
      Beam::MemberAccessExpression get_size() const;

      /** Returns an accessor for the market center member. */
      Beam::MemberAccessExpression get_market_center() const;

      /** Returns an accessor for the buyer MPID member. */
      Beam::MemberAccessExpression get_buyer_mpid() const;

      /** Returns an accessor for the seller MPID member. */
      Beam::MemberAccessExpression get_seller_mpid() const;

    private:
      Beam::Expression m_expression;
  };

  inline TimeAndSaleAccessor::TimeAndSaleAccessor(
    Beam::Expression expression) noexcept
    : m_expression(std::move(expression)) {}

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_timestamp() const {
    return Beam::MemberAccessExpression(
      "timestamp", typeid(boost::posix_time::ptime), m_expression);
  }

  inline Beam::MemberAccessExpression TimeAndSaleAccessor::get_price() const {
    return Beam::MemberAccessExpression("price", typeid(Money), m_expression);
  }

  inline Beam::MemberAccessExpression TimeAndSaleAccessor::get_size() const {
    return Beam::MemberAccessExpression("size", typeid(Quantity), m_expression);
  }

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_market_center() const {
    return Beam::MemberAccessExpression(
      "market_center", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_buyer_mpid() const {
    return Beam::MemberAccessExpression(
      "buyer_mpid", typeid(std::string), m_expression);
  }

  inline Beam::MemberAccessExpression
      TimeAndSaleAccessor::get_seller_mpid() const {
    return Beam::MemberAccessExpression(
      "seller_mpid", typeid(std::string), m_expression);
  }
}

#endif
