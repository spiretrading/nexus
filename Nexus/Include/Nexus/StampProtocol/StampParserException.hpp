#ifndef NEXUS_STAMPPARSEREXCEPTION_HPP
#define NEXUS_STAMPPARSEREXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/StampProtocol/StampProtocol.hpp"

namespace Nexus {
namespace StampProtocol {

  /*! \class StampParserException
      \brief Exception used to indicate that a parsing operation failed.
   */
  class StampParserException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a StampParserException.
      StampParserException();

      //! Constructs a StampParserException.
      /*!
        \param message A message describing the error.
      */
      StampParserException(const std::string& message);

      virtual ~StampParserException() throw();
  };

  inline StampParserException::StampParserException()
      : std::runtime_error("Operation failed") {}

  inline StampParserException::StampParserException(
      const std::string& message)
      : std::runtime_error(message) {}

  inline StampParserException::~StampParserException() throw() {}
}
}

#endif
