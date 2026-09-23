#ifndef OTC_LINK_PARSER_EXCEPTION_HPP
#define OTC_LINK_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Indicates that an OTC Link parsing operation failed. */
  class OtcLinkParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an OtcLinkParserException with a default message. */
      OtcLinkParserException();
  };

  inline OtcLinkParserException::OtcLinkParserException()
    : OtcLinkParserException("OTC Link parser failed.") {}
}

#endif
