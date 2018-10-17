#ifndef BEAM_RECORDPARSER_HPP
#define BEAM_RECORDPARSER_HPP
#include <Beam/Parsers/BasicParser.hpp>
#include <Beam/Pointers/Ref.hpp>
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class RecordParser
      \brief Implements a Parser for a Record.
   */
  class RecordParser : public Beam::Parsers::BasicParser<Record> {
    public:

      //! Constructs a RecordParser.
      /*!
        \param recordType The type of Record to parse.
        \param userProfile The user's profile.
      */
      RecordParser(const RecordType& recordType,
        Beam::Ref<UserProfile> userProfile);
  };
}

#endif
