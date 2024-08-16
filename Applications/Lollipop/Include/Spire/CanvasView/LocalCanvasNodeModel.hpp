#ifndef SPIRE_LOCAL_CANVAS_NODE_MODEL_HPP
#define SPIRE_LOCAL_CANVAS_NODE_MODEL_HPP
#include <unordered_map>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"

namespace Spire {

  /** Implements a CanvasNodeModel by directly keeping track of node states. */
  class LocalCanvasNodeModel : public CanvasNodeModel {
    public:
      std::vector<const CanvasNode*> GetRoots() const override;

      boost::optional<const CanvasNode&>
        GetNode(const Coordinate& coordinate) const override;

      Coordinate GetCoordinate(const CanvasNode& node) const override;

      boost::optional<Coordinate> GetCurrentCoordinate() const override;

      void SetCurrent(const Coordinate& coordinate) override;

      const CanvasNode& Add(
        const Coordinate& coordinate, const CanvasNode& node) override;

      void Remove(const CanvasNode& node) override;

    private:
      std::vector<std::unique_ptr<CanvasNode>> m_roots;
      Coordinate m_current;
      std::unordered_map<Coordinate, const CanvasNode*> m_coordinateToNode;
      std::unordered_map<const CanvasNode*, Coordinate> m_nodeToCoordinate;

      void InternalAdd(const Coordinate& coordinate, const CanvasNode& node);
      void InternalRemove(const CanvasNode& node);
  };
}

#endif
