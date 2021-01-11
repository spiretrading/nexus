#ifndef SPIRE_DEFAULTCANVASNODEFROMCANVASTYPEVISITOR_HPP
#define SPIRE_DEFAULTCANVASNODEFROMCANVASTYPEVISITOR_HPP
#include <memory>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CanvasTypeVisitor.hpp"

namespace Spire {

  //! Returns the default CanvasNode to use for a specified CanvasType.
  /*!
    \param type The CanvasType to get the CanvasNode from.
    \return The default CanvasNode for the specified <i>type</i>.
  */
  std::unique_ptr<CanvasNode> MakeDefaultCanvasNode(const CanvasType& type);
}

#endif
