#ifndef SPIRE_PARSERTYPES_HPP
#define SPIRE_PARSERTYPES_HPP
#include <Beam/Parsers/BoolParser.hpp>
#include <Beam/Parsers/DateTimeParser.hpp>
#include <Beam/Parsers/DecimalParser.hpp>
#include <Beam/Parsers/IntegralParser.hpp>
#include <Beam/Parsers/StringParser.hpp>
#include <Beam/Parsers/TimeDurationParser.hpp>
#include "Nexus/Parsers/CurrencyParser.hpp"
#include "Nexus/Parsers/MarketParser.hpp"
#include "Nexus/Parsers/MoneyParser.hpp"
#include "Nexus/Parsers/OrderStatusParser.hpp"
#include "Nexus/Parsers/OrderTypeParser.hpp"
#include "Nexus/Parsers/SecurityParser.hpp"
#include "Nexus/Parsers/SideParser.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Records/RecordParser.hpp"

namespace Spire {

  /*! \struct ParserType
      \brief Returns the type of Parser used for a specified type.
      \tparam T The type to get the Parser for.
   */
  template<typename T>
  struct ParserType {};

  template<>
  struct ParserType<bool> {
    typedef Beam::Parsers::BoolParser type;
  };

  template<>
  struct ParserType<Nexus::Quantity> {
    typedef Beam::Parsers::IntegralParser<Nexus::Quantity> type;
  };

  template<>
  struct ParserType<double> {
    typedef Beam::Parsers::DecimalParser<double> type;
  };

  template<>
  struct ParserType<boost::posix_time::ptime> {
    typedef Beam::Parsers::DateTimeParser type;
  };

  template<>
  struct ParserType<boost::posix_time::time_duration> {
    typedef Beam::Parsers::TimeDurationParser type;
  };

  template<>
  struct ParserType<std::string> {
    typedef Beam::Parsers::StringParser type;
  };

  template<>
  struct ParserType<Task::State> {
    typedef TaskStateParser type;
  };

  template<>
  struct ParserType<Nexus::CurrencyId> {
    typedef Nexus::CurrencyParser type;
  };

  template<>
  struct ParserType<Nexus::MarketCode> {
    typedef Nexus::MarketParser type;
  };

  template<>
  struct ParserType<Nexus::Money> {
    typedef Nexus::MoneyParser type;
  };

  template<>
  struct ParserType<Nexus::OrderStatus> {
    typedef Nexus::OrderStatusParser type;
  };

  template<>
  struct ParserType<Nexus::OrderType> {
    typedef Nexus::OrderTypeParser type;
  };

  template<>
  struct ParserType<Nexus::Security> {
    typedef Nexus::SecurityParser type;
  };

  template<>
  struct ParserType<Nexus::Side> {
    typedef Nexus::SideParser type;
  };

  template<>
  struct ParserType<Nexus::TimeInForce> {
    typedef Nexus::TimeInForceParser type;
  };

  template<>
  struct ParserType<Record> {
    typedef RecordParser type;
  };
}

#endif
