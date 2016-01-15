#ifndef NEXUS_MARKETPARSER_HPP
#define NEXUS_MARKETPARSER_HPP
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /*! \class MarketParser
      \brief Matches a MarketCode.
   */
  class MarketParser : public Beam::Parsers::EnumeratorParser<MarketCode> {
    public:

      //! Constructs a MarketParser.
      /*!
        \param marketDatabase The database of markets to match against.
      */
      MarketParser(const MarketDatabase& marketDatabase);

    private:
      typedef boost::transform_iterator<
        MarketCode (*)(const MarketDatabase::Entry&),
        std::vector<MarketDatabase::Entry>::const_iterator> MarketCodeIterator;
      static MarketCode GetCode(const MarketDatabase::Entry& entry);
  };

  inline MarketParser::MarketParser(const MarketDatabase& marketDatabase)
      : Beam::Parsers::EnumeratorParser<MarketCode>(
          MarketCodeIterator(marketDatabase.GetEntries().cbegin(), &GetCode),
          MarketCodeIterator(marketDatabase.GetEntries().cend(), &GetCode),
          [&] (MarketCode code) {
            return marketDatabase.FromCode(code).m_displayName;
          }) {}

  inline MarketCode MarketParser::GetCode(const MarketDatabase::Entry& entry) {
    return entry.m_code;
  }
}

#endif
