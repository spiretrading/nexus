#ifndef SPIRE_CANVASOPERATIONEXCEPTION_HPP
#define SPIRE_CANVASOPERATIONEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class CanvasOperationException
      \brief Indicates an operation on a canvas failed.
   */
  class CanvasOperationException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a CanvasOperationException.
      CanvasOperationException();

      //! Constructs a CanvasOperationException.
      /*!
        \param message Indicates the reason for the failure.
      */
      CanvasOperationException(std::string message);

      virtual ~CanvasOperationException() throw();
  };
}

#endif
