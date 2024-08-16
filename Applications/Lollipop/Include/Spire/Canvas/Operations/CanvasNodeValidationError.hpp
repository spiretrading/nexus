#ifndef SPIRE_CANVASNODEVALIDATIONERROR_HPP
#define SPIRE_CANVASNODEVALIDATIONERROR_HPP
#include <string>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class CanvasNodeValidationError
      \brief Indicates that a CanvasNode is not well formed.
   */
  class CanvasNodeValidationError {
    public:

      //! Constructs a CanvasNodeValidationError.
      CanvasNodeValidationError();

      //! Constructs a CanvasNodeValidationError.
      /*!
        \param node The CanvasNode that is invalid.
        \param message The error message.
      */
      CanvasNodeValidationError(Beam::Ref<const CanvasNode> node,
        std::string message);

      //! Returns the invalid CanvasNode.
      const CanvasNode& GetNode() const;

      //! Returns the error message.
      const std::string& GetErrorMessage() const;

    private:
      const CanvasNode* m_node;
      std::string m_message;
  };
}

#endif
