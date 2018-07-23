#ifndef SPIRE_CANVASNODEVALIDATOR_HPP
#define SPIRE_CANVASNODEVALIDATOR_HPP
#include <vector>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidationError.hpp"

namespace Spire {

  //! Validates a CanvasNode.
  /*!
    \param node The CanvasNode to validate.
    \return The list of errors founds in the <i>node</i>.
  */
  std::vector<CanvasNodeValidationError> Validate(const CanvasNode& node);
}

#endif
