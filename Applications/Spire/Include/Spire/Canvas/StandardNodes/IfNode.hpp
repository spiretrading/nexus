#ifndef SPIRE_IFNODE_HPP
#define SPIRE_IFNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies an IfNode's signatures.
  struct IfNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<bool, T, T, T>::type type;
    };

    typedef boost::mpl::transform<NativeTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };

  /*! \class IfNode
      \brief Evaluates to one of two CanvasNodes depending on a condition.
   */
  class IfNode : public FunctionNode {
    public:

      //! Constructs an IfNode.
      IfNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      IfNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void IfNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::IfNode> : std::false_type {};
}
}

#endif
