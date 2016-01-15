#ifndef SPIRE_ISTERMINALNODE_HPP
#define SPIRE_ISTERMINALNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class IsTerminalNode
      \brief Tests whether a Task State represents a terminal state.
   */
  class IsTerminalNode : public CanvasNode {
    public:

      //! Constructs an IsTerminalNode.
      IsTerminalNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      IsTerminalNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void IsTerminalNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::IsTerminalNode> : std::false_type {};
}
}

#endif
