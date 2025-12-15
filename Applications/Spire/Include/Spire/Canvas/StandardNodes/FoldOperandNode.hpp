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
      friend struct Beam::DataShuttle;
      Side m_side;

      FoldOperandNode() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void FoldOperandNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
    shuttle.shuttle("side", m_side);
  }
}

#endif
