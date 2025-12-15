#ifndef NEXUS_PITCH_PARSER_EXCEPTION_HPP
#define NEXUS_PITCH_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Exception used to indicate that a parsing operation failed. */
  class PitchParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a PitchParserException with a default message. */
      PitchParserException();
  };

  inline PitchParserException::PitchParserException()
    : PitchParserException("PITCH parser failed.") {}
}

#endif
