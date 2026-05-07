#ifndef NEXUS_OTC_LINK_PARSER_EXCEPTION_HPP
#define NEXUS_OTC_LINK_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Indicates that parsing an OTC Link message failed. */
  class OtcLinkParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a OtcLinkParserException. */
      OtcLinkParserException();
  };

  inline OtcLinkParserException::OtcLinkParserException()
    : OtcLinkParserException("OTC Link parsing operation failed.") {}
}

#endif
