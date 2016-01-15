#ifndef NEXUS_SOUPBINTCPDATATYPES_HPP
#define NEXUS_SOUPBINTCPDATATYPES_HPP
#include <cstdint>
#include <string>
#include <Beam/Pointers/Out.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/SoupBinTcp/SoupBinTcp.hpp"

namespace Nexus {
namespace SoupBinTcp {

  //! Parses a left-padded alpha-numeric field.
  /*!
    \param length The length of the field.
    \param cursor The first character of the field.
    \return A string containing the field.
  */
  inline std::string ParseLeftPaddedAlphaNumeric(std::size_t length,
      Beam::Out<const char*> cursor) {
    auto token = *cursor;
    auto remainingLength = length;
    while(remainingLength > 0 && *token == ' ') {
      ++token;
      --remainingLength;
    }
    *cursor += length;
    std::string result(token, *cursor);
    return result;
  }

  //! Parses a left-padded numeric field.
  /*!
    \param length The length of the field.
    \param cursor The first character of the field.
    \return The field's numeric value.
  */
  template<typename T>
  T ParseLeftPaddedNumeric(std::size_t length, Beam::Out<const char*> cursor) {
    auto value = ParseLeftPaddedAlphaNumeric(length, Beam::Store(cursor));
    return boost::lexical_cast<T>(value);
  }
}
}

#endif
