#ifndef SPIRE_DECIMALNODE_HPP
#define SPIRE_DECIMALNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/DecimalType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class DecimalNode
      \brief Implements the CanvasNode for a double value.
   */
  class DecimalNode : public ValueNode<DecimalType> {
    public:

      //! Constructs a DecimalNode.
      DecimalNode();

      //! Constructs a DecimalNode.
      /*!
        \param value The initial value.
      */
      DecimalNode(double value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<DecimalNode> SetValue(double value) const;

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
  void DecimalNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<DecimalType>::shuttle(shuttle, version);
  }
}

#endif
