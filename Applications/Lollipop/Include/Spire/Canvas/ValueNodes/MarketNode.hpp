#ifndef SPIRE_MARKETNODE_HPP
#define SPIRE_MARKETNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/MarketType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class MarketNode
      \brief Implements the CanvasNode for the MarketType.
   */
  class MarketNode : public ValueNode<MarketType> {
    public:

      //! Constructs a MarketNode.
      MarketNode();

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<MarketNode> SetValue(Nexus::MarketCode value) const;

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
  void MarketNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<MarketType>::Shuttle(shuttle, version);
  }
}

#endif
