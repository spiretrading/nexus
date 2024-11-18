#ifndef NEXUS_QUERYSTANDARDDATATYPES_HPP
#define NEXUS_QUERYSTANDARDDATATYPES_HPP
#include <boost/mpl/list.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/variant/variant.hpp>
#include <Beam/Queries/MemberAccessExpression.hpp>
#include <Beam/Queries/StandardDataTypes.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/MarketQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus::Queries {
  using QuantityType = Beam::Queries::NativeDataType<Quantity>;
  using MoneyType = Beam::Queries::NativeDataType<Money>;
  using SecurityType = Beam::Queries::NativeDataType<Security>;
  using SecurityInfoType = Beam::Queries::NativeDataType<SecurityInfo>;
  using OrderImbalanceType = Beam::Queries::NativeDataType<OrderImbalance>;
  using BboQuoteType = Beam::Queries::NativeDataType<BboQuote>;
  using BookQuoteType = Beam::Queries::NativeDataType<BookQuote>;
  using MarketQuoteType = Beam::Queries::NativeDataType<MarketQuote>;
  using TimeAndSaleType = Beam::Queries::NativeDataType<TimeAndSale>;
  using OrderFieldsType = Beam::Queries::NativeDataType<
    OrderExecutionService::OrderFields>;
  using OrderInfoType = Beam::Queries::NativeDataType<
    OrderExecutionService::OrderInfo>;
  using QueryVariant = boost::variant<bool, char, int, double, std::uint64_t,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration,
    Quantity, Money, Security, SecurityInfo, OrderImbalance, BboQuote,
    BookQuote, MarketQuote, TimeAndSale, OrderExecutionService::OrderFields,
    OrderExecutionService::OrderInfo>;
  using SequencedQueryVariant = Beam::Queries::SequencedValue<QueryVariant>;

  /**
   * Provides member accessors for an Expression evaluating to a TimeAndSale.
   */
  struct TimeAndSaleAccessor {

    /**
     * Constructs a TimeAndSaleAccessor.
     * @param expression The expression whose members are to be accessed.
     */
    explicit TimeAndSaleAccessor(Beam::Queries::Expression expression);

    Beam::Queries::MemberAccessExpression m_timestamp;
    Beam::Queries::MemberAccessExpression m_price;
    Beam::Queries::MemberAccessExpression m_size;
    Beam::Queries::MemberAccessExpression m_marketCenter;
    Beam::Queries::MemberAccessExpression m_buyerMpid;
    Beam::Queries::MemberAccessExpression m_sellerMpid;
  };

  struct QueryTypes {
    using ExtendedNativeTypes = boost::mpl::list<Quantity, Money, Security,
      SecurityInfo, OrderImbalance, BboQuote, BookQuote, MarketQuote,
      TimeAndSale, OrderExecutionService::OrderFields,
      OrderExecutionService::OrderInfo>;
    using NativeTypes = boost::mpl::insert_range<
      Beam::Queries::QueryTypes::NativeTypes,
      boost::mpl::end<Beam::Queries::QueryTypes::NativeTypes>::type,
      ExtendedNativeTypes>::type;
    using ExtendedValueTypes = boost::mpl::list<Quantity, Money>;
    using ValueTypes = boost::mpl::insert_range<
      Beam::Queries::QueryTypes::ValueTypes,
      boost::mpl::end<Beam::Queries::QueryTypes::ValueTypes>::type,
      ExtendedValueTypes>::type;
    using ExtendedComparableTypes = boost::mpl::list<Quantity, Money>;
    using ComparableTypes = boost::mpl::insert_range<
      Beam::Queries::QueryTypes::ComparableTypes,
      boost::mpl::end<Beam::Queries::QueryTypes::ComparableTypes>::type,
      ExtendedComparableTypes>::type;
  };

  inline TimeAndSaleAccessor::TimeAndSaleAccessor(
    Beam::Queries::Expression expression)
    : m_timestamp("timestamp", Beam::Queries::DateTimeType(), expression),
      m_price("price", MoneyType(), expression),
      m_size("size", QuantityType(), expression),
      m_marketCenter("market_center", Beam::Queries::StringType(), expression),
      m_buyerMpid("buyer_mpid", Beam::Queries::StringType(), expression),
      m_sellerMpid("seller_mpid", Beam::Queries::StringType(), expression) {}
}

#endif
