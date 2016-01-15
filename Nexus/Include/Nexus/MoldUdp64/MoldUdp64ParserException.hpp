#ifndef NEXUS_MOLDUDP64PARSEREXCEPTION_HPP
#define NEXUS_MOLDUDP64PARSEREXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/MoldUdp64/MoldUdp64.hpp"

namespace Nexus {
namespace MoldUdp64 {

  /*! \class MoldUdp64ParserException
      \brief Indicates that parsing an MoldUDP64 message failed.
   */
  class MoldUdp64ParserException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs an MoldUdp64ParserException.
      MoldUdp64ParserException();

      //! Constructs a MoldUdp64ParserException.
      /*!
        \param message A message describing the error.
      */
      MoldUdp64ParserException(const std::string& message);

      virtual ~MoldUdp64ParserException() throw();
  };

  inline MoldUdp64ParserException::MoldUdp64ParserException()
      : std::runtime_error("Operation failed") {}

  inline MoldUdp64ParserException::MoldUdp64ParserException(
      const std::string& message)
      : std::runtime_error(message) {}

  inline MoldUdp64ParserException::~MoldUdp64ParserException() throw() {}
}
}

#endif
