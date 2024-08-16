#ifndef SPIRE_CANVASNODENOTVISIBLEEXCEPTION_HPP
#define SPIRE_CANVASNODENOTVISIBLEEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class CanvasNodeNotVisibleException
      \brief Indicates an CanvasNode was added to a table but has no visible
             nodes.
   */
  class CanvasNodeNotVisibleException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a CanvasNodeNotVisibleException.
      CanvasNodeNotVisibleException();

      //! Constructs a CanvasNodeNotVisibleException.
      /*!
        \param message Indicates the reason for the failure.
      */
      CanvasNodeNotVisibleException(const std::string& message);

      virtual ~CanvasNodeNotVisibleException() throw();
  };
}

#endif
