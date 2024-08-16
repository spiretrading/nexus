#ifndef SPIRE_CURRENCYNODE_HPP
#define SPIRE_CURRENCYNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CurrencyType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class CurrencyNode
      \brief Implements the CanvasNode for the CurrencyType.
   */
  class CurrencyNode : public ValueNode<CurrencyType> {
    public:

      //! Constructs a CurrencyNode.
      CurrencyNode();

      //! Constructs a CurrencyNode with an initial value.
      /*!
        \param value The new value.
        \param code The currency code to display.
      */
      CurrencyNode(Nexus::CurrencyId value, std::string code);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \param code The currency code to display.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<CurrencyNode> SetValue(Nexus::CurrencyId value,
        std::string code) const;

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
  void CurrencyNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<CurrencyType>::Shuttle(shuttle, version);
  }
}

#endif
