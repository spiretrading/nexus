#ifndef SPIRE_CANVASTYPENOTFOUNDEXCEPTION_HPP
#define SPIRE_CANVASTYPENOTFOUNDEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class CanvasTypeNotFoundException
      \brief Indicates a CanvasType was not found.
   */
  class CanvasTypeNotFoundException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a CanvasTypeNotFoundException.
      CanvasTypeNotFoundException();

      //! Constructs a CanvasTypeNotFoundException.
      /*!
        \param message Indicates the reason for the failure.
      */
      CanvasTypeNotFoundException(std::string message);

      virtual ~CanvasTypeNotFoundException() throw();
  };
}

#endif
