#ifndef SPIRE_LAST_NODE_HPP
#define SPIRE_LAST_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a LastNode's signatures.
  struct LastNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T>::type type;
    };

    typedef boost::mpl::transform<NativeTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };

  /*! \class LastNode
      \brief Evaluates to the last value it receives.
   */
  class LastNode : public FunctionNode {
    public:

      //! Constructs a LastNode.
      LastNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      LastNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void LastNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::LastNode> : std::false_type {};
}
}

#endif
