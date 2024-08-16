#ifndef SPIRE_GREATEROREQUALSNODE_HPP
#define SPIRE_GREATEROREQUALSNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  /*! \class GreaterOrEqualsNode
      \brief Tests if one value is greater than or equal to another.
   */
  class GreaterOrEqualsNode : public FunctionNode {
    public:

      //! Constructs a GreaterOrEqualsNode.
      GreaterOrEqualsNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      GreaterOrEqualsNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void GreaterOrEqualsNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::GreaterOrEqualsNode> :
    std::false_type {};
}
}

#endif
