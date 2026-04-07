#ifndef NEXUS_CURRENCY_PARSER_HPP
#define NEXUS_CURRENCY_PARSER_HPP
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {

  /**
   * Parses a CurrencyId.
   * @param currencies The database of available currencies to parse.
   */
  inline auto currency_parser(const CurrencyDatabase& currencies) {
    auto id = [] (const auto& value) {
      return value.m_id;
    };
    return Beam::EnumeratorParser(
      boost::make_transform_iterator(currencies.get_entries().cbegin(), id),
      boost::make_transform_iterator(currencies.get_entries().cend(), id),
      [=] (auto id) {
        return boost::lexical_cast<std::string>(currencies.from(id).m_code);
      });
  }

  /** Parses a CurrencyCode using the default CurrencyDatabase. */
  inline const auto& currency_parser() {
    static const auto& parser = currency_parser(DEFAULT_CURRENCIES);
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::CurrencyId> = Nexus::currency_parser();
}

#endif
