#ifndef SPIRE_RANGENODE_HPP
#define SPIRE_RANGENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"

namespace Spire {

  //! Specifies a RangeNode's signatures.
  struct RangeNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T, T>::type type;
    };

    typedef boost::mpl::transform<boost::mpl::list<Nexus::Quantity>,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };

  /*! \class RangeNode
      \brief Produces a range of values.
   */
  class RangeNode : public FunctionNode {
    public:

      //! Constructs a RangeNode.
      RangeNode();

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      RangeNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void RangeNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::RangeNode> : std::false_type {};
}
}

#endif
