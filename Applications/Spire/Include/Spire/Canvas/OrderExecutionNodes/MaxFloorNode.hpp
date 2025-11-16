#ifndef SPIRE_MAXFLOORNODE_HPP
#define SPIRE_MAXFLOORNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/LinkedNode.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class MaxFloorNode
      \brief Represents an Order's max floor option.
   */
  class MaxFloorNode : public ValueNode<IntegerType>, public LinkedNode {
    public:

      //! Constructs a MaxFloorNode.
      MaxFloorNode();

      //! Constructs a MaxFloorNode.
      /*!
        \param value The initial value.
      */
      MaxFloorNode(Nexus::Quantity value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<MaxFloorNode> SetValue(Nexus::Quantity value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      virtual const std::string& GetReferent() const;

      virtual std::unique_ptr<CanvasNode> SetReferent(
        const std::string& referent) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;
      std::string m_referent;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void MaxFloorNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<IntegerType>::shuttle(shuttle, version);
    shuttle.shuttle("referent", m_referent);
  }
}

#endif
