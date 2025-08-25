#ifndef NEXUS_QUERYSTANDARDDATATYPES_HPP
#define NEXUS_QUERYSTANDARDDATATYPES_HPP
#include <boost/mpl/list.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/variant/variant.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/StandardDataTypes.hpp>
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
  using QuantityType = Beam::Queries::NativeDataType<Quantity>;
  using MoneyType = Beam::Queries::NativeDataType<Money>;
  using SecurityType = Beam::Queries::NativeDataType<Security>;
  using SecurityInfoType = Beam::Queries::NativeDataType<SecurityInfo>;
  using OrderImbalanceType = Beam::Queries::NativeDataType<OrderImbalance>;
  using BboQuoteType = Beam::Queries::NativeDataType<BboQuote>;
  using BookQuoteType = Beam::Queries::NativeDataType<BookQuote>;
  using TimeAndSaleType = Beam::Queries::NativeDataType<TimeAndSale>;
  using OrderFieldsType = Beam::Queries::NativeDataType<OrderFields>;
  using OrderInfoType = Beam::Queries::NativeDataType<OrderInfo>;
  using QueryVariant = boost::variant<bool, char, int, double, std::uint64_t,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration,
    Quantity, Money, Security, SecurityInfo, OrderImbalance, BboQuote,
    BookQuote, TimeAndSale, OrderFields, OrderInfo>;
  using SequencedQueryVariant = Beam::Queries::SequencedValue<QueryVariant>;

  struct QueryTypes {
    using ExtendedNativeTypes = boost::mpl::list<Quantity, Money, Security,
      SecurityInfo, OrderImbalance, BboQuote, BookQuote, TimeAndSale,
      OrderFields, OrderInfo>;
    using NativeTypes =
      boost::mpl::insert_range<Beam::Queries::QueryTypes::NativeTypes,
        boost::mpl::end<Beam::Queries::QueryTypes::NativeTypes>::type,
        ExtendedNativeTypes>::type;
    using ExtendedValueTypes = boost::mpl::list<Quantity, Money>;
    using ValueTypes =
      boost::mpl::insert_range<Beam::Queries::QueryTypes::ValueTypes,
        boost::mpl::end<Beam::Queries::QueryTypes::ValueTypes>::type,
        ExtendedValueTypes>::type;
    using ExtendedComparableTypes = boost::mpl::list<Quantity, Money>;
    using ComparableTypes =
      boost::mpl::insert_range<Beam::Queries::QueryTypes::ComparableTypes,
        boost::mpl::end<Beam::Queries::QueryTypes::ComparableTypes>::type,
        ExtendedComparableTypes>::type;
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
      explicit TimeAndSaleAccessor(Beam::Queries::Expression expression);

      /** Returns an accessor for the timestamp member. */
      Beam::Queries::MemberAccessExpression get_timestamp() const;

      /** Returns an accessor for the price member. */
      Beam::Queries::MemberAccessExpression get_price() const;

      /** Returns an accessor for the size member. */
      Beam::Queries::MemberAccessExpression get_size() const;

      /** Returns an accessor for the market center member. */
      Beam::Queries::MemberAccessExpression get_market_center() const;

      /** Returns an accessor for the buyer MPID member. */
      Beam::Queries::MemberAccessExpression get_buyer_mpid() const;

      /** Returns an accessor for the seller MPID member. */
      Beam::Queries::MemberAccessExpression get_seller_mpid() const;

    private:
      Beam::Queries::Expression m_expression;
  };

  inline TimeAndSaleAccessor::TimeAndSaleAccessor(
    Beam::Queries::Expression expression)
    : m_expression(std::move(expression)) {}

  inline Beam::Queries::MemberAccessExpression
      TimeAndSaleAccessor::get_timestamp() const {
    return Beam::Queries::MemberAccessExpression(
      "timestamp", Beam::Queries::DateTimeType(), m_expression);
  }

  inline Beam::Queries::MemberAccessExpression
      TimeAndSaleAccessor::get_price() const {
    return Beam::Queries::MemberAccessExpression(
      "price", MoneyType(), m_expression);
  }

  inline Beam::Queries::MemberAccessExpression
      TimeAndSaleAccessor::get_size() const {
    return Beam::Queries::MemberAccessExpression(
      "size", QuantityType(), m_expression);
  }

  inline Beam::Queries::MemberAccessExpression
      TimeAndSaleAccessor::get_market_center() const {
    return Beam::Queries::MemberAccessExpression(
      "market_center", Beam::Queries::StringType(), m_expression);
  }

  inline Beam::Queries::MemberAccessExpression
      TimeAndSaleAccessor::get_buyer_mpid() const {
    return Beam::Queries::MemberAccessExpression(
      "buyer_mpid", Beam::Queries::StringType(), m_expression);
  }

  inline Beam::Queries::MemberAccessExpression
      TimeAndSaleAccessor::get_seller_mpid() const {
    return Beam::Queries::MemberAccessExpression(
      "seller_mpid", Beam::Queries::StringType(), m_expression);
  }
}

#endif
