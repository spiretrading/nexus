#ifndef NEXUS_COUNTRY_PARSER_HPP
#define NEXUS_COUNTRY_PARSER_HPP
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Country.hpp"

namespace Nexus {

  /**
   * Parses a CountryCode.
   * @param countries The database used to lookup country codes.
   */
  inline auto country_parser(const CountryDatabase& countries) {
    auto code = [] (const auto& entry) {
      return entry.m_code;
    };
    return Beam::EnumeratorParser(
      boost::make_transform_iterator(countries.get_entries().cbegin(), code),
      boost::make_transform_iterator(countries.get_entries().cend(), code),
      [=] (auto code) {
        return boost::lexical_cast<std::string>(
          countries.from(code).m_three_letter_code);
      });
  }

  /** Parses a CountryCode using the default CountryDatabase. */
  inline const auto& country_parser() {
    static const auto& parser = country_parser(DEFAULT_COUNTRIES);
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::CountryCode> = Nexus::country_parser();
}

#endif
