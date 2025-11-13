#ifndef SPIRE_FOLDNODE_HPP
#define SPIRE_FOLDNODE_HPP
#include <boost/mp11.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/FunctionNode.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

namespace Spire {

  //! Specifies a FoldNode's signatures.
  struct FoldSignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<T, T, T>;

    using type = boost::mp11::mp_transform<MakeSignature, ValueTypes>;
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
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void FoldNode::shuttle(S& shuttle, unsigned int version) {
    FunctionNode::shuttle(shuttle, version);
  }
}

#endif
