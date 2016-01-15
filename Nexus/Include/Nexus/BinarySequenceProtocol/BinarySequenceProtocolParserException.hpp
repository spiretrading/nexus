#ifndef NEXUS_BINARYSEQUENCEPROTOCOLPARSEREXCEPTION_HPP
#define NEXUS_BINARYSEQUENCEPROTOCOLPARSEREXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocol.hpp"

namespace Nexus {
namespace BinarySequenceProtocol {

  /*! \class BinarySequenceProtocolParserException
      \brief Indicates that parsing a message failed.
   */
  class BinarySequenceProtocolParserException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a BinarySequenceProtocolParserException.
      BinarySequenceProtocolParserException();

      //! Constructs a BinarySequenceProtocolParserException.
      /*!
        \param message A message describing the error.
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
}

#endif
