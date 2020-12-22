#ifndef SPIRE_TASKVOLUMENODE_HPP
#define SPIRE_TASKVOLUMENODE_HPP
#include <memory>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Return a CanvasNode that represents the quantity executed by an Order
  //! Task.
  std::unique_ptr<CanvasNode> MakeTaskVolumeNode();
}

#endif
