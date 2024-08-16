#ifndef SPIRE_ORDERTYPENODE_HPP
#define SPIRE_ORDERTYPENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/OrderTypeType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class OrderTypeNode
      \brief Implements the CanvasNode for an OrderType value.
   */
  class OrderTypeNode : public ValueNode<OrderTypeType> {
    public:

      //! Constructs a OrderTypeNode.
      OrderTypeNode();

      //! Constructs a OrderTypeNode.
      /*!
        \param value The initial value.
      */
      OrderTypeNode(Nexus::OrderType value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<OrderTypeNode> SetValue(Nexus::OrderType value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void OrderTypeNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<OrderTypeType>::Shuttle(shuttle, version);
  }
}

#endif
