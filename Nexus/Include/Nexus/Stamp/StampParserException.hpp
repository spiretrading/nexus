#ifndef NEXUS_STAMP_PARSER_EXCEPTION_HPP
#define NEXUS_STAMP_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Exception used to indicate that a parsing operation failed. */
  class StampParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a StampParserException. */
      StampParserException();
  };

  inline StampParserException::StampParserException()
    : StampParserException("Operation failed") {}
}

#endif
