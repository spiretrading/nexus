#ifndef SPIRE_STATICNODE_HPP
#define SPIRE_STATICNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a StaticNode's signatures.
  struct StaticNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T>::type type;
    };

    typedef boost::mpl::transform<ValueTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };

  /*! \class StaticNode
      \brief Evaluates to the first value it receives.
   */
  class StaticNode : public FunctionNode {
    public:

      //! Constructs a StaticNode.
      StaticNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      StaticNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void StaticNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::StaticNode> : std::false_type {};
}
}

#endif
