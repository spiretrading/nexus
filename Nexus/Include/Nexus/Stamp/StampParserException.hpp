#ifndef NEXUS_STAMP_PARSER_EXCEPTION_HPP
#define NEXUS_STAMP_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Indicates that parsing a STAMP message failed. */
  class StampParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a StampParserException. */
      StampParserException();
  };

  inline StampParserException::StampParserException()
    : StampParserException("Invalid STAMP message.") {}
}

#endif
