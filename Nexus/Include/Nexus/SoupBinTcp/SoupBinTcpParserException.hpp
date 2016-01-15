#ifndef NEXUS_SOUPBINTCPPARSEREXCEPTION_HPP
#define NEXUS_SOUPBINTCPPARSEREXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/SoupBinTcp/SoupBinTcp.hpp"

namespace Nexus {
namespace SoupBinTcp {

  /*! \class SoupBinTcpParserException
      \brief Indicates that parsing a SoupBinTcp packet failed.
   */
  class SoupBinTcpParserException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a SoupBinTcpParserException.
      SoupBinTcpParserException();

      //! Constructs a SoupBinTcpParserException.
      /*!
        \param message A message describing the error.
      */
      SoupBinTcpParserException(const std::string& message);

      virtual ~SoupBinTcpParserException() throw();
  };

  inline SoupBinTcpParserException::SoupBinTcpParserException()
      : std::runtime_error("Operation failed") {}

  inline SoupBinTcpParserException::SoupBinTcpParserException(
      const std::string& message)
      : std::runtime_error(message) {}

  inline SoupBinTcpParserException::~SoupBinTcpParserException() throw() {}
}
}

#endif
