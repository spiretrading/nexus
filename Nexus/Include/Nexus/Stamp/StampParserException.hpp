#ifndef NEXUS_STAMP_PARSER_EXCEPTION_HPP
#define NEXUS_STAMP_PARSER_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace Nexus::Stamp {

  /** Exception used to indicate that a parsing operation failed. */
  class StampParserException :
      public std::runtime_error, public boost::exception {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a StampParserException. */
      StampParserException();
  };

  inline StampParserException::StampParserException()
    : std::runtime_error("Operation failed") {}
}

#endif
