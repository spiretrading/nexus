#ifndef NEXUS_QUERY_STANDARD_DATA_TYPES_HPP
#define NEXUS_QUERY_STANDARD_DATA_TYPES_HPP
#include <cstdint>
#include <type_traits>
#include <variant>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <Beam/Queries/SequencedValue.hpp>
#include <boost/mp11/list.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/BookQuote.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderImbalance.hpp"
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Nexus/Definitions/TickerStatus.hpp"
#include "Nexus/Queries/OrderFieldsAccessor.hpp"
#include "Nexus/Queries/OrderInfoAccessor.hpp"
#include "Nexus/Queries/TickerAccessor.hpp"
#include "Nexus/Queries/TimeAndSaleAccessor.hpp"

namespace Beam {
  template<>
  struct is_compatible_operand<Nexus::Quantity, int> : std::true_type {};

  template<>
  struct is_compatible_operand<int, Nexus::Quantity> : std::true_type {};

  template<>
  struct is_compatible_operand<Nexus::Quantity, double> : std::true_type {};

  template<>
  struct is_compatible_operand<double, Nexus::Quantity> : std::true_type {};

  template<>
  struct is_compatible_operand<Nexus::Quantity, std::uint64_t> :
    std::true_type {};

  template<>
  struct is_compatible_operand<std::uint64_t, Nexus::Quantity> :
    std::true_type {};
}

namespace Nexus {
  using QueryVariant = std::variant<bool, char, int, double, std::uint64_t,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration,
    Quantity, Money, Side, Quote, Venue, Ticker, TickerInfo, OrderImbalance,
    BboQuote, BookQuote, TickerStatus, TimeAndSale, OrderFields, OrderInfo,
    AccountModificationRequest>;
  using SequencedQueryVariant = Beam::SequencedValue<QueryVariant>;

  struct QueryTypes {
    using ExtendedNativeTypes = boost::mp11::mp_list<Quantity, Money, Side,
      Quote, Venue, Ticker, TickerInfo, OrderImbalance, BboQuote, BookQuote,
      TickerStatus, TimeAndSale, OrderFields, OrderInfo,
      AccountModificationRequest>;
    using NativeTypes = boost::mp11::mp_append<
      Beam::QueryTypes::NativeTypes, ExtendedNativeTypes>;
    using ExtendedValueTypes =
      boost::mp11::mp_list<Quantity, Money, Side, Venue>;
    using ValueTypes =
      boost::mp11::mp_append<Beam::QueryTypes::ValueTypes, ExtendedValueTypes>;
    using ExtendedComparableTypes = boost::mp11::mp_list<Quantity, Money>;
    using ComparableTypes = boost::mp11::mp_append<
      Beam::QueryTypes::ComparableTypes, ExtendedComparableTypes>;
  };

namespace Details {
  inline const auto QUERY_PROMOTIONS = Beam::register_promotions<QueryTypes>();
}
}

#endif
