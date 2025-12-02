#ifndef SPIRE_TEXTNODE_HPP
#define SPIRE_TEXTNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/TextType.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

namespace Spire {

  /*! \class TextNode
      \brief Implements the CanvasNode for a string value.
   */
  class TextNode : public ValueNode<TextType> {
    public:

      //! Constructs a TextNode.
      TextNode();

      //! Constructs a TextNode.
      /*!
        \param value The initial value.
      */
      TextNode(std::string value);

      //! Clones this CanvasNode with a new value.
      /*!
        \param value The new value.
        \return A clone of this CanvasNode with the specified <i>value</i>.
      */
      std::unique_ptr<TextNode> SetValue(std::string value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::DataShuttle;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TextNode::shuttle(S& shuttle, unsigned int version) {
    ValueNode<TextType>::shuttle(shuttle, version);
  }
}

#endif
