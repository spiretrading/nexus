#ifndef SPIRE_RESOLVEREFERENCES_HPP
#define SPIRE_RESOLVEREFERENCES_HPP
#include <memory>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Resolves the CanvasType's of all references in a CanvasNode.
  /*!
    \param node The CanvasNode whose references are to be resolved.
    \return A new CanvasNode with its reference's CanvasType's resolved.
  */
  std::unique_ptr<CanvasNode> ResolveReferences(
    std::unique_ptr<CanvasNode> node);
}

#endif
