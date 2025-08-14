#ifndef NEXUS_SOUP_BIN_TCP_PARSER_EXCEPTION_HPP
#define NEXUS_SOUP_BIN_TCP_PARSER_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace Nexus::SoupBinTcp {

  /** Indicates that parsing a SoupBinTcp packet failed. */
  class SoupBinTcpParserException :
      public std::runtime_error, public boost::exception {
    public:

      /** Constructs a SoupBinTcpParserException. */
      SoupBinTcpParserException();

      /**
       * Constructs a SoupBinTcpParserException.
       * @param message A message describing the error.
       */
      explicit SoupBinTcpParserException(const std::string& message);
  };

  inline SoupBinTcpParserException::SoupBinTcpParserException()
    : std::runtime_error("Operation failed") {}

  inline SoupBinTcpParserException::SoupBinTcpParserException(
    const std::string& message)
    : std::runtime_error(message) {}
}

#endif
