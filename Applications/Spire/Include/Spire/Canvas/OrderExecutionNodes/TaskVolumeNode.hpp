#ifndef SPIRE_TASK_VOLUME_NODE_HPP
#define SPIRE_TASK_VOLUME_NODE_HPP
#include <memory>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /**
   * Return a CanvasNode that represents the quantity executed by an Order
   * Task.
   */
  std::unique_ptr<CanvasNode> MakeTaskVolumeNode();
}

#endif
