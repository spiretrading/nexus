#ifndef NEXUS_BINARY_SEQUENCE_PROTOCOL_PARSER_EXCEPTION_HPP
#define NEXUS_BINARY_SEQUENCE_PROTOCOL_PARSER_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocol.hpp"

namespace Nexus::BinarySequenceProtocol {

  /** Indicates that parsing a message failed. */
  class BinarySequenceProtocolParserException : public std::runtime_error,
      public boost::exception {
    public:

      /** Constructs a BinarySequenceProtocolParserException. */
      BinarySequenceProtocolParserException();

      /**
       * Constructs a BinarySequenceProtocolParserException.
       * @param message A message describing the error.
       */
      BinarySequenceProtocolParserException(const std::string& message);

      virtual ~BinarySequenceProtocolParserException() throw();
  };

  inline BinarySequenceProtocolParserException::
    BinarySequenceProtocolParserException()
    : std::runtime_error("Operation failed") {}

  inline BinarySequenceProtocolParserException::
    BinarySequenceProtocolParserException(const std::string& message)
    : std::runtime_error(message) {}

  inline BinarySequenceProtocolParserException::
    ~BinarySequenceProtocolParserException() throw() {}
}

#endif
