#ifndef SPIRE_CHAINNODE_HPP
#define SPIRE_CHAINNODE_HPP
#include <vector>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class ChainNode
      \brief Evaluates a series of CanvasNodes in order.
   */
  class ChainNode : public CanvasNode {
    public:

      //! Constructs a ChainNode.
      ChainNode();

      //! Constructs a ChainNode.
      /*!
        \param nodes The list of CanvasNodes to chain.
      */
      ChainNode(std::vector<std::unique_ptr<CanvasNode>> nodes);

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      ChainNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ChainNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::ChainNode> : std::false_type {};
}
}

#endif
