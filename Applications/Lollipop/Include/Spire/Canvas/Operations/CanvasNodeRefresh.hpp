#ifndef SPIRE_CANVASNODEREFRESH_HPP
#define SPIRE_CANVASNODEREFRESH_HPP
#include <memory>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Refreshes a CanvasNode.
  /*!
    \param node The CanvasNode to refresh.
    \return A CanvasNode whose signatures have been reset.
  */
  std::unique_ptr<CanvasNode> Refresh(std::unique_ptr<CanvasNode> node);
}

#endif
