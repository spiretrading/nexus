#ifndef SPIRE_PROXYNODE_HPP
#define SPIRE_PROXYNODE_HPP
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"

namespace Spire {

  /*! \class ProxyNode
      \brief Masks one CanvasNode with a reference to another.
   */
  class ProxyNode : public ReferenceNode {
    public:

      //! Constructs a ProxyNode.
      /*!
        \param referent The path to the CanvasNode being referred.
        \param type The CanvasType to evaluate to.
        \param original The original CanvasNode being masked.
      */
      ProxyNode(const std::string& referent, const CanvasType& type,
        std::unique_ptr<CanvasNode> original);

      //! Returns the original CanvasNode being masked.
      const CanvasNode& GetOriginal() const;

      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      virtual std::unique_ptr<CanvasNode> SetVisible(bool value) const;

      virtual std::unique_ptr<CanvasNode> SetReadOnly(bool value) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::shared_ptr<CanvasNode> m_original;

      ProxyNode() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ProxyNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    ReferenceNode::Shuttle(shuttle, version);
    shuttle.Shuttle("original", m_original);
  }
}

#endif
