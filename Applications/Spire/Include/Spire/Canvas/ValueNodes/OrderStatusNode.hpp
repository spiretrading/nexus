#ifndef SPIRE_ORDERSTATUSNODE_HPP
#define SPIRE_ORDERSTATUSNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/OrderStatusType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class OrderStatusNode
      \brief Implements the CanvasNode for an OrderStatus value.
   */
  class OrderStatusNode : public ValueNode<OrderStatusType> {
    public:

      //! Constructs a OrderStatusNode.
      OrderStatusNode();

      //! Constructs a OrderStatusNode.
      /*!
        \param value The initial value.
      */
      OrderStatusNode(Nexus::OrderStatus value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<OrderStatusNode> SetValue(Nexus::OrderStatus value) const;

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
  void OrderStatusNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<OrderStatusType>::Shuttle(shuttle, version);
  }
}

#endif
