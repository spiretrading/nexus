#ifndef TMX_IP_PARSER_EXCEPTION_HPP
#define TMX_IP_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Reports a TMX IP parsing failure. */
  class TmxIpParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a TmxIpParserException. */
      TmxIpParserException();
  };

  inline TmxIpParserException::TmxIpParserException()
    : TmxIpParserException("TMX IP parser failed.") {}
}

#endif
