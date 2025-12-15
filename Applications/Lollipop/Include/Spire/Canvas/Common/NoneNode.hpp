#ifndef SPIRE_NONENODE_HPP
#define SPIRE_NONENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class NoneNode
      \brief Represents no value.
   */
  class NoneNode : public CanvasNode {
    public:

      //! Constructs a NoneNode.
      NoneNode();

      //! Constructs a NoneNode.
      /*!
        \param type The type to evaluate to.
      */
      NoneNode(const CanvasType& type);

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void NoneNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
  }
}

#endif
