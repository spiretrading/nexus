#ifndef NEXUS_COUNTRYPARSER_HPP
#define NEXUS_COUNTRYPARSER_HPP
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /**
   * Parses a CountryCode.
   * @param countryDatabase The database used to lookup country codes.
   */
  inline auto CountryParser(const CountryDatabase& countryDatabase) {
    auto code = [] (const auto& entry) {
      return entry.m_code;
    };
    return Beam::Parsers::EnumeratorParser(boost::make_transform_iterator(
      countryDatabase.GetEntries().cbegin(), code),
      boost::make_transform_iterator(countryDatabase.GetEntries().cend(), code),
      [&] (auto code) {
        return boost::lexical_cast<std::string>(
          countryDatabase.FromCode(code).m_threeLetterCode);
      });
  }

  /** Parses a CountryCode using the default CountryDatabase. */
  inline const auto& CountryParser() {
    static const auto& parser = CountryParser(GetDefaultCountryDatabase());
    return parser;
  }
}

#endif
