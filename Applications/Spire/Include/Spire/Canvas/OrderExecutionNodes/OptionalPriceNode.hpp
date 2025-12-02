#ifndef SPIRE_OPTIONALPRICENODE_HPP
#define SPIRE_OPTIONALPRICENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/LinkedNode.hpp"
#include "Spire/Canvas/Types/MoneyType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class OptionalPriceNode
      \brief Used when a price is only optional.
   */
  class OptionalPriceNode : public ValueNode<MoneyType>, public LinkedNode {
    public:

      //! Constructs an OptionalPriceNode.
      OptionalPriceNode();

      //! Constructs an OptionalPriceNode.
      /*!
        \param value The initial value.
      */
      OptionalPriceNode(Nexus::Money value);

      //! Returns the reference price.
      Nexus::Money GetReferencePrice() const;

      //! Clones this CanvasNode with a new reference price.
      /*!
        \param referencePrice The new reference price.
        \return A clone of this CanvasNode with the specified
                <i>referencePrice</i>.
      */
      std::unique_ptr<OptionalPriceNode> SetReferencePrice(
        Nexus::Money referencePrice) const;

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<OptionalPriceNode> SetValue(Nexus::Money value) const;

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
      Nexus::Money m_referencePrice;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void OptionalPriceNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<MoneyType>::shuttle(shuttle, version);
    shuttle.shuttle("referent", m_referent);
    shuttle.shuttle("reference_price", m_referencePrice);
  }
}

#endif
