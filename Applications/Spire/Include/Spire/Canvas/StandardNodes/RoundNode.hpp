#ifndef SPIRE_ROUNDNODE_HPP
#define SPIRE_ROUNDNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /*! \class RoundNode
      \brief Rounds a CanvasNode.
   */
  class RoundNode : public FunctionNode {
    public:

      //! Constructs a RoundNode.
      RoundNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      RoundNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void RoundNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::RoundNode> : std::false_type {};
}
}

#endif
