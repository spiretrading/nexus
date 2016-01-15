#ifndef SPIRE_IGNORECOMMANDEXCEPTION_HPP
#define SPIRE_IGNORECOMMANDEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {
namespace UI {

  /*! \class IgnoreCommandException
      \brief Used to avoid adding a failed QUndoCommand to the QUndoStack.
   */
  class IgnoreCommandException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a IgnoreCommandException.
      IgnoreCommandException();

      //! Constructs a IgnoreCommandException.
      /*!
        \param message Indicates the reason for the failure.
      */
      IgnoreCommandException(const std::string& message);

      virtual ~IgnoreCommandException() throw();
  };
}
}

#endif
