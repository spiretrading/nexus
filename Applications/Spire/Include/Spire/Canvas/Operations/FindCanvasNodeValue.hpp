#ifndef SPIRE_FIND_CANVAS_NODE_HPP
#define SPIRE_FIND_CANVAS_NODE_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /**
   * Finds the native value of the first child with a matching name.
   * @param node The node to start the search from.
   * @param name The name of the node to find.
   * @return The value associated with the matching node, or <i>none</i> iff
   *          the node isn't found or the type of node doesn't match.
   */
  template<typename T>
  boost::optional<typename T::Type::Type> find_value(
      const CanvasNode& node, const std::string& name) {
    if(auto base = node.FindChild(name)) {
      if(auto node = dynamic_cast<const T*>(&*base)) {
        return node->GetValue();
      }
    }
    return boost::none;
  }
}

#endif
