#ifndef NEXUS_COUNTRYPARSER_HPP
#define NEXUS_COUNTRYPARSER_HPP
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /*! \class CountryParser
      \brief Matches a CountryCode.
   */
  class CountryParser : public Beam::Parsers::EnumeratorParser<CountryCode> {
    public:

      //! Constructs a CountryParser.
      /*!
        \param countryDatabase The database of countries to match against.
      */
      CountryParser(const CountryDatabase& countryDatabase);

    private:
      typedef boost::transform_iterator<
        CountryCode (*)(const CountryDatabase::Entry&),
        std::vector<CountryDatabase::Entry>::const_iterator>
        CountryCodeIterator;
      static CountryCode GetCode(const CountryDatabase::Entry& entry);
  };

  inline CountryParser::CountryParser(const CountryDatabase& countryDatabase)
      : Beam::Parsers::EnumeratorParser<CountryCode>(
          CountryCodeIterator(countryDatabase.GetEntries().cbegin(), &GetCode),
          CountryCodeIterator(countryDatabase.GetEntries().cend(), &GetCode),
          [&] (CountryCode code) {
            return ToString(countryDatabase.FromCode(code).m_threeLetterCode);
          }) {}

  inline CountryCode CountryParser::GetCode(
      const CountryDatabase::Entry& entry) {
    return entry.m_code;
  }
}

#endif
