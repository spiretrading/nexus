#ifndef SPIRE_FOLDOPERANDNODE_HPP
#define SPIRE_FOLDOPERANDNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class FoldOperandNode
      \brief Represents one of the operands of a FoldNode.
   */
  class FoldOperandNode : public CanvasNode {
    public:

      /*! \enum Side
          \brief Enumerates the sides of a FoldNode.
       */
      enum class Side {

        //! The left hand side of the operation.
        LEFT,

        //! The right hand side of the operation.
        RIGHT
      };

      //! Constructs a FoldOperandNode.
      /*!
        \param side The side this operand represents.
      */
      FoldOperandNode(Side side);

      //! Copies a FoldOperandNode.
      /*!
        \param node The node to clone.
      */
      FoldOperandNode(const FoldOperandNode& node) = default;

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      Side m_side;

      FoldOperandNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void FoldOperandNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("side", m_side);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::FoldOperandNode> : std::false_type {};
}
}

#endif
