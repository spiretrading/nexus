#ifndef SPIRE_SIDENODE_HPP
#define SPIRE_SIDENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/SideType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class SideNode
      \brief Implements the CanvasNode for a Side value.
   */
  class SideNode : public ValueNode<SideType> {
    public:

      //! Constructs a SideNode.
      SideNode();

      //! Constructs a SideNode.
      /*!
        \param value The initial value.
      */
      SideNode(Nexus::Side value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<SideNode> SetValue(Nexus::Side value) const;

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
  void SideNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ValueNode<SideType>::Shuttle(shuttle, version);
  }
}

#endif
