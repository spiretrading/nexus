#ifndef SPIRE_MINNODE_HPP
#define SPIRE_MINNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /*! \class MinNode
      \brief Returns the lesser of two CanvasNodes.
   */
  class MinNode : public FunctionNode {
    public:

      //! Constructs a MinNode.
      MinNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      MinNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void MinNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::MinNode> : std::false_type {};
}
}

#endif
