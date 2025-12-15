#ifndef NEXUS_SOUP_BIN_TCP_PARSER_EXCEPTION_HPP
#define NEXUS_SOUP_BIN_TCP_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Indicates that parsing a SoupBinTcp packet failed. */
  class SoupBinTcpParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a SoupBinTcpParserException. */
      SoupBinTcpParserException();
  };

  inline SoupBinTcpParserException::SoupBinTcpParserException()
    : SoupBinTcpParserException("Operation failed") {}
}

#endif
