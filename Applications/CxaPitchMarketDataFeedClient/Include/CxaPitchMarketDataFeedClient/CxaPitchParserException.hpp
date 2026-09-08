#ifndef CXA_PITCH_PARSER_EXCEPTION_HPP
#define CXA_PITCH_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Exception used to indicate that a parsing operation failed. */
  class CxaPitchParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a CxaPitchParserException with a default message. */
      CxaPitchParserException();
  };

  inline CxaPitchParserException::CxaPitchParserException()
    : CxaPitchParserException("CXA PITCH parser failed.") {}
}

#endif
