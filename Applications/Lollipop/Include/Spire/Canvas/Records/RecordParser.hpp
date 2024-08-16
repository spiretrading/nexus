#ifndef BEAM_RECORDPARSER_HPP
#define BEAM_RECORDPARSER_HPP
#include <Beam/Pointers/Ref.hpp>
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Returns a RecordParser.
  /*!
    \param recordType The type of Record to parse.
    \param userProfile The user's profile.
  */
  Beam::Parsers::RuleParser<Record> RecordParser(const RecordType& recordType,
    Beam::Ref<UserProfile> userProfile);
}

#endif
