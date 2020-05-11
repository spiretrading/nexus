#ifndef NEXUS_MARKETPARSER_HPP
#define NEXUS_MARKETPARSER_HPP
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /**
   * Parses a MarketCode.
   * @param marketDatabase The database used to lookup market codes.
   */
  inline auto MarketParser(const MarketDatabase& marketDatabase) {
    auto code = [] (const auto& entry) {
      return entry.m_code;
    };
    return Beam::Parsers::EnumeratorParser(boost::make_transform_iterator(
      marketDatabase.GetEntries().cbegin(), code),
      boost::make_transform_iterator(marketDatabase.GetEntries().cend(), code),
      [&] (auto code) {
        return marketDatabase.FromCode(code).m_displayName;
      });
  }

  /** Parses a MarketCode using the default MarketDatabase. */
  inline const auto& MarketParser() {
    static const auto& parser = MarketParser(GetDefaultMarketDatabase());
    return parser;
  }
}

#endif
