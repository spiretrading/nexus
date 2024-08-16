#ifndef SPIRE_ABSNODE_HPP
#define SPIRE_ABSNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies an AbsNode's signatures.
  struct AbsNodeSignatures {
    typedef boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, Nexus::Quantity>,
      boost::mpl::vector<double, double>,
      boost::mpl::vector<Nexus::Money, Nexus::Money>> type;
  };

  /*! \class AbsNode
      \brief Evaluates the absolute value of two CanvasNodes.
   */
  class AbsNode : public FunctionNode {
    public:

      //! Constructs an AbsNode.
      AbsNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      AbsNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void AbsNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::AbsNode> : std::false_type {};
}
}

#endif
