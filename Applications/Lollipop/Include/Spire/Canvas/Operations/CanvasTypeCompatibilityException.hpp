#ifndef SPIRE_CANVASTYPECOMPATIBILITYEXCEPTION_HPP
#define SPIRE_CANVASTYPECOMPATIBILITYEXCEPTION_HPP
#include "Spire/Canvas/Operations/CanvasOperationException.hpp"

namespace Spire {

  /*! \class CanvasTypeCompatibilityException
      \brief Indicates an operation failed due to incompatible types.
   */
  class CanvasTypeCompatibilityException : public CanvasOperationException {
    public:

      //! Constructs a CanvasTypeIncompatibilityException.
      CanvasTypeCompatibilityException();

      //! Constructs a CanvasTypeIncompatibilityException.
      /*!
        \param message Indicates the reason for the failure.
      */
      CanvasTypeCompatibilityException(std::string message);

      virtual ~CanvasTypeCompatibilityException() throw();
  };
}

#endif
