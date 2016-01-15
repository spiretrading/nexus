#ifndef NEXUS_CURRENCYPARSER_HPP
#define NEXUS_CURRENCYPARSER_HPP
#include <Beam/Parsers/EnumeratorParser.hpp>
#include <Beam/Parsers/SubParserStream.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Parsers/Parsers.hpp"

namespace Nexus {

  /*! \class CurrencyParser
      \brief Matches a CurrencyId.
   */
  class CurrencyParser : public Beam::Parsers::EnumeratorParser<CurrencyId> {
    public:

      //! Constructs a CurrencyParser.
      /*!
        \param currencyDatabase The database of currencies to match against.
      */
      CurrencyParser(const CurrencyDatabase& currencyDatabase);

    private:
      typedef boost::transform_iterator<
        CurrencyId (*)(const CurrencyDatabase::Entry&),
        std::vector<CurrencyDatabase::Entry>::const_iterator>
        CurrencyIdIterator;
      static CurrencyId GetId(const CurrencyDatabase::Entry& entry);
  };

  inline CurrencyParser::CurrencyParser(
      const CurrencyDatabase& currencyDatabase)
      : Beam::Parsers::EnumeratorParser<CurrencyId>(
          CurrencyIdIterator(currencyDatabase.GetEntries().cbegin(), &GetId),
          CurrencyIdIterator(currencyDatabase.GetEntries().cend(), &GetId),
          [&] (CurrencyId id) {
            return ToString(currencyDatabase.FromId(id).m_code);
          }) {}

  inline CurrencyId CurrencyParser::GetId(
      const CurrencyDatabase::Entry& entry) {
    return entry.m_id;
  }
}

#endif
