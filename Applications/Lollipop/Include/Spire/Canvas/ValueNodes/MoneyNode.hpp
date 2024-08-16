#ifndef SPIRE_MONEYNODE_HPP
#define SPIRE_MONEYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/LinkedNode.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class MoneyNode
      \brief Implements the CanvasNode for a Money value.
   */
  class MoneyNode : public ValueNode<MoneyType>, public LinkedNode {
    public:

      //! Constructs a MoneyNode.
      MoneyNode();

      //! Constructs a MoneyNode.
      /*!
        \param value The initial value.
      */
      MoneyNode(Nexus::Money value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<MoneyNode> SetValue(Nexus::Money value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      virtual const std::string& GetReferent() const;

      virtual std::unique_ptr<CanvasNode> SetReferent(
        const std::string& referent) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_referent;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void MoneyNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<MoneyType>::Shuttle(shuttle, version);
    shuttle.Shuttle("referent", m_referent);
  }
}

#endif
