#ifndef NEXUS_QUERY_STANDARD_DATA_TYPES_HPP
#define NEXUS_QUERY_STANDARD_DATA_TYPES_HPP
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <boost/mp11/list.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Queries/OrderFieldsAccessor.hpp"
#include "Nexus/Queries/OrderInfoAccessor.hpp"
#include "Nexus/Queries/SecurityAccessor.hpp"
#include "Nexus/Queries/TimeAndSaleAccessor.hpp"

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
    using NativeTypes = boost::mp11::mp_append<
      Beam::QueryTypes::NativeTypes, ExtendedNativeTypes>;
    using ExtendedValueTypes = boost::mp11::mp_list<Quantity, Money>;
    using ValueTypes =
      boost::mp11::mp_append<Beam::QueryTypes::ValueTypes, ExtendedValueTypes>;
    using ExtendedComparableTypes = boost::mp11::mp_list<Quantity, Money>;
    using ComparableTypes = boost::mp11::mp_append<
      Beam::QueryTypes::ComparableTypes, ExtendedComparableTypes>;
  };
}

#endif
