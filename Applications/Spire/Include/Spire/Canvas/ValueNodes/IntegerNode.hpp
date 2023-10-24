#ifndef SPIRE_INTEGER_NODE_HPP
#define SPIRE_INTEGER_NODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/LinkedNode.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /** Implements the CanvasNode for a Quantity value. */
  class IntegerNode : public ValueNode<IntegerType>, public LinkedNode {
    public:

      /** Constructs an IntegerNode representing the value 0. */
      IntegerNode();

      /**
       * Constructs an IntegerNode.
       * @param value The initial value.
       */
      IntegerNode(Nexus::Quantity value);

      /**
       * Clones this CanvasNode with a new value.
       * \param value The new value.
       * @return A clone of this CanvasNode with the specified <i>value</i>.
       */
      std::unique_ptr<IntegerNode> SetValue(int value) const;

      void Apply(CanvasNodeVisitor& visitor) const override;

      const std::string& GetReferent() const override;

      std::unique_ptr<CanvasNode>
        SetReferent(const std::string& referent) const override;

    protected:
      std::unique_ptr<CanvasNode> Clone() const override;

      std::unique_ptr<CanvasNode> Reset() const override;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_referent;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void IntegerNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<IntegerType>::Shuttle(shuttle, version);
    shuttle.Shuttle("referent", m_referent);
  }
}

#endif
