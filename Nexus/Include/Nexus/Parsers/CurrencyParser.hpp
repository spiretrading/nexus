#ifndef NEXUS_CURRENCYPARSER_HPP
#define NEXUS_CURRENCYPARSER_HPP
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /**
   * Parses a CurrencyId.
   * @param countryDatabase The database used to lookup country codes.
   */
  inline auto CurrencyParser(const CurrencyDatabase& database) {
    auto id = [] (const auto& value) {
      return value.m_id;
    };
    return Beam::Parsers::EnumeratorParser(boost::make_transform_iterator(
      database.GetEntries().cbegin(), id),
      boost::make_transform_iterator(database.GetEntries().cend(), id),
      [&] (auto id) {
        return boost::lexical_cast<std::string>(database.FromId(id).m_code);
      });
  }

  /** Parses a CurrencyCode using the default CurrencyDatabase. */
  inline const auto& CurrencyParser() {
    static const auto& parser = CurrencyParser(GetDefaultCurrencyDatabase());
    return parser;
  }
}

#endif
