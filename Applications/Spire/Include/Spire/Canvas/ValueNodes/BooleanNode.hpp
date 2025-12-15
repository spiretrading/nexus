#ifndef SPIRE_BOOLEANNODE_HPP
#define SPIRE_BOOLEANNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/BooleanType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class BooleanNode
      \brief Implements the CanvasNode for a bool value.
   */
  class BooleanNode : public ValueNode<BooleanType> {
    public:

      //! Constructs a BooleanNode.
      BooleanNode();

      //! Constructs a BooleanNode.
      /*!
        \param value The initial value.
      */
      BooleanNode(bool value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<BooleanNode> SetValue(bool value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BooleanNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<BooleanType>::shuttle(shuttle, version);
  }
}

#endif
