#ifndef SPIRE_REFERENCENODE_HPP
#define SPIRE_REFERENCENODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/LinkedNode.hpp"

namespace Spire {

  /*! \class ReferenceNode
      \brief References another CanvasNode.
   */
  class ReferenceNode : public CanvasNode, public LinkedNode {
    public:

      //! Constructs a ReferenceNode.
      ReferenceNode();

      //! Constructs a ReferenceNode.
      /*!
        \param referent The path to the CanvasNode being referenced.
      */
      ReferenceNode(const std::string& referent);

      //! Constructs a ReferenceNode.
      /*!
        \param referent The path to the CanvasNode being referenced.
        \param type The CanvasType to evaluate to.
      */
      ReferenceNode(const std::string& referent, const CanvasType& type);

      virtual const std::string& GetReferent() const;

      virtual std::unique_ptr<CanvasNode> SetReferent(
        const std::string& referent) const;

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct Beam::DataShuttle;
      std::string m_referent;
  };

  //! Returns the result of repeatedly getting a ReferenceNode's referent.
  /*!
    \param node The CanvasNode whose source is to be extracted.
    \return The result of repeatedly getting a ReferenceNode's referent.
  */
  boost::optional<const CanvasNode&> FindAnchor(const CanvasNode& node);

  template<Beam::IsShuttle S>
  void ReferenceNode::shuttle(S& shuttle, unsigned int version) {
    CanvasNode::shuttle(shuttle, version);
    shuttle.shuttle("referent", m_referent);
  }
}

#endif
