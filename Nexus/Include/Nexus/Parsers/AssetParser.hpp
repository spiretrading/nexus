#ifndef NEXUS_ASSET_PARSER_HPP
#define NEXUS_ASSET_PARSER_HPP
#include <Beam/Parsers/ConversionParser.hpp>
#include <Beam/Parsers/DefaultParser.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/Definitions/Asset.hpp"
#include "Nexus/Definitions/Currency.hpp"

namespace Nexus {

  /**
   * Parses an Asset.
   * @param currencies The database of available currencies to parse.
   */
  inline auto asset_parser(const CurrencyDatabase& currencies) {
    auto id = [] (const auto& value) {
      return value.m_id;
    };
    return Beam::cast<Asset>(Beam::EnumeratorParser(
      boost::make_transform_iterator(currencies.get_entries().cbegin(), id),
      boost::make_transform_iterator(currencies.get_entries().cend(), id),
      [=] (auto id) {
        return boost::lexical_cast<std::string>(currencies.from(id).m_code);
      }));
  }

  /** Parses an Asset using the default CurrencyDatabase. */
  inline const auto& asset_parser() {
    static const auto& parser = asset_parser(DEFAULT_CURRENCIES);
    return parser;
  }
}

namespace Beam {
  template<>
  const auto default_parser<Nexus::Asset> = Nexus::asset_parser();
}

#endif
