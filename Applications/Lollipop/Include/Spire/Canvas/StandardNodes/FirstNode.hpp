#ifndef SPIRE_FIRST_NODE_HPP
#define SPIRE_FIRST_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a FirstNode's signatures.
  struct FirstNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T>::type type;
    };

    typedef boost::mpl::transform<NativeTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };

  /*! \class FirstNode
      \brief Evaluates to the first value it receives.
   */
  class FirstNode : public FunctionNode {
    public:

      //! Constructs a FirstNode.
      FirstNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      FirstNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void FirstNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::FirstNode> : std::false_type {};
}
}

#endif
