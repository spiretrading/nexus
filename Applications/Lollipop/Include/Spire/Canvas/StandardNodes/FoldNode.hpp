#ifndef SPIRE_FOLDNODE_HPP
#define SPIRE_FOLDNODE_HPP
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

namespace Spire {

  //! Specifies a FoldNode's signatures.
  struct FoldSignatures {
    template<typename T>
    struct MakeSignature {
      using type = typename boost::mpl::vector<T, T, T>::type;
    };

    using type = boost::mpl::transform<ValueTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type;
  };

  /*! \class FoldNode
      \brief A CanvasNode that performs a fold operation on a data stream.
   */
  class FoldNode : public FunctionNode {
    public:

      //! Constructs a FoldNode.
      FoldNode();

      //! Returns the CanvasNode that takes the place of the combiner's left
      //! operand.
      boost::optional<const CanvasNode&> FindLeftOperand() const;

      //! Returns the CanvasNode that takes the place of the combiner's right
      //! operand.
      boost::optional<const CanvasNode&> FindRightOperand() const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      FoldNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void FoldNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    FunctionNode::Shuttle(shuttle, version);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::FoldNode> : std::false_type {};
}
}

#endif
