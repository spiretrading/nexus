#ifndef NEXUS_MOLD_UDP_64_PARSER_EXCEPTION_HPP
#define NEXUS_MOLD_UDP_64_PARSER_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace Nexus::MoldUdp64 {

  /** Indicates that parsing an MoldUDP64 message failed. */
  class MoldUdp64ParserException :
      public std::runtime_error, public boost::exception {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a MoldUdp64ParserException. */
      MoldUdp64ParserException();
  };

  inline MoldUdp64ParserException::MoldUdp64ParserException()
    : std::runtime_error("Operation failed") {}
}

#endif
