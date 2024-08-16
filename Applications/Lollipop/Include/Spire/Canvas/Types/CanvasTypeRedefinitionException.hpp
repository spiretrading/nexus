#ifndef SPIRE_CANVASTYPEREDEFINITIONEXCEPTION_HPP
#define SPIRE_CANVASTYPEREDEFINITIONEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class CanvasTypeRedefinitionException
      \brief Indicates a CanvasType already has a definition.
   */
  class CanvasTypeRedefinitionException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a CanvasTypeRedefinitionException.
      CanvasTypeRedefinitionException();

      //! Constructs a CanvasTypeRedefinitionException.
      /*!
        \param message Indicates the reason for the failure.
      */
      CanvasTypeRedefinitionException(std::string message);

      virtual ~CanvasTypeRedefinitionException() throw();
  };
}

#endif
