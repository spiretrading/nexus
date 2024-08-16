#ifndef SPIRE_CUSTOMNODE_HPP
#define SPIRE_CUSTOMNODE_HPP
#include <memory>
#include <string>
#include <vector>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class CustomNode
      \brief An extensible node that evaluates to its first child.
   */
  class CustomNode : public CanvasNode {
    public:

      /*! \struct Child
          \brief Stores information about a child of this CustomNode.
       */
      struct Child {

        //! The name of the child.
        std::string m_name;

        //! The type that the child must be compatible with.
        std::shared_ptr<CanvasType> m_type;

        //! Constructs an empty Child.
        Child();

        //! Copies a Child.
        Child(const Child& child) = default;

        //! Constructs a Child.
        /*!
          \param name The name of the child.
          \param type The type that the child must be compatible with.
        */
        Child(std::string name, const CanvasType& type);

        template<typename Shuttler>
        void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      //! Constructs a CustomNode.
      /*!
        \param name The name of this CustomNode.
        \param children The children of this CustomNode.
      */
      CustomNode(std::string name, std::vector<Child> children);

      //! Returns the details of the children nodes.
      const std::vector<Child>& GetChildrenDetails() const;

      virtual boost::optional<const CanvasNode&> FindNode(
        const std::string& name) const;

      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

      using CanvasNode::Replace;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::vector<Child> m_children;

      CustomNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CustomNode::Child::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("type", m_type);
  }

  template<typename Shuttler>
  void CustomNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("children", m_children);
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsDefaultConstructable<Spire::CustomNode> : std::false_type {};
}
}

#endif
