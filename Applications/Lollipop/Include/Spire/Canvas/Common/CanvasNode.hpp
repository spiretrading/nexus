#ifndef SPIRE_CANVASNODE_HPP
#define SPIRE_CANVASNODE_HPP
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <Beam/Collections/View.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Utilities/Casts.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class CanvasNode
      \brief Base class for a node within a canvas.
   */
  class CanvasNode {
    public:

      //! Defines the types of meta-data that can be stored in a CanvasNode.
      typedef boost::variant<bool, Nexus::Quantity, double,
        boost::posix_time::ptime, boost::posix_time::time_duration, std::string,
        Nexus::CurrencyId, Nexus::Money, Nexus::OrderType, Nexus::Ticker,
        Nexus::Side, Nexus::TimeInForce, Nexus::Venue> MetaData;

      //! A CanvasNode's MIME type.
      static const std::string MIME_TYPE;

      //! Clones a CanvasNode.
      /*!
        \param node The CanvasNode to clone.
        \return A clone of the <i>node</i>.
      */
      template<typename T>
      static std::unique_ptr<T> Clone(const T& node);

      virtual ~CanvasNode();

      //! Returns the parent.
      boost::optional<const CanvasNode&> GetParent() const;

      //! Returns the children.
      Beam::View<const CanvasNode> GetChildren() const;

      //! Returns the type that this node evaluates to.
      const CanvasType& GetType() const;

      //! Returns the text representation of the node.
      const std::string& GetText() const;

      //! Returns the name of this node.
      const std::string& GetName() const;

      //! Returns <code>true</code> iff this node is visible.
      bool IsVisible() const;

      //! Returns <code>true</code> iff this node is read-only.
      bool IsReadOnly() const;

      //! Returns the meta-data with a specified name.
      /*!
        \param name The name of the meta-data to find.
        \return The meta-data with the specified <i>name</i>.
      */
      boost::optional<const MetaData&> FindMetaData(
        const std::string& name) const;

      //! Finds a child with a specified name.
      /*!
        \param name The name of the child to find.
        \return The CanvasNode with the specified <i>name</i>.
      */
      virtual boost::optional<const CanvasNode&> FindChild(
        const std::string& name) const;

      //! Finds a CanvasNode with a specified name.
      /*!
        \param name The name of the CanvasNode to find.
        \return The CanvasNode with the specified <i>name</i>.
      */
      virtual boost::optional<const CanvasNode&> FindNode(
        const std::string& name) const;

      //! Returns a new CanvasNode from this whose type has been converted.
      /*!
        \param type The CanvasType to convert this instance to.
        \return A CanvasNode whose type has been converted.
      */
      virtual std::unique_ptr<CanvasNode> Convert(const CanvasType& type) const;

      //! Returns a CanvasNode whose child has been replaced.
      /*!
        \param child The name of the child to replace.
        \param replacement The replacement CanvasNode.
        \return A CanvasNode whose <i>child</i> has been replaced with
                <i>replacement</i>.
      */
      virtual std::unique_ptr<CanvasNode> Replace(const std::string& child,
        std::unique_ptr<CanvasNode> replacement) const;

      //! Returns a CanvasNode whose child has been replaced.
      /*!
        \param child The child to replace.
        \param replacement The replacement CanvasNode.
        \return A CanvasNode whose <i>child</i> has been replaced with
                <i>replacement</i>.
      */
      virtual std::unique_ptr<CanvasNode> Replace(const CanvasNode& child,
        std::unique_ptr<CanvasNode> replacement) const;

      //! Sets whether this node is visible.
      /*!
        \param isVisible <code>true</code> to set this node as visible.
        \return A CanvasNode with the specified visibility.
      */
      virtual std::unique_ptr<CanvasNode> SetVisible(bool isVisible) const;

      //! Sets whether this node is read-only.
      /*!
        \param isReadOnly <code>true</code> to set this node as read-only.
        \return A CanvasNode with the specified read-only property.
      */
      virtual std::unique_ptr<CanvasNode> SetReadOnly(bool isReadOnly) const;

      //! Sets the meta-data with a given name.
      /*!
        \param name The name of the meta-data to set.
        \param value The meta-data's value.
        \return A CanvasNode with the specified meta data.
      */
      virtual std::unique_ptr<CanvasNode> SetMetaData(const std::string& name,
        MetaData value) const;

      //! Deletes meta-data with a given name.
      /*!
        \param name The name of the meta-data to delete.
        \return A CanvasNode with the specified meta data deleted.
      */
      virtual std::unique_ptr<CanvasNode> DeleteMetaData(
        const std::string& name) const;

      //! Applies a CanvasNodeVisitor to this instance.
      /*!
        \param visitor The CanvasNodeVisitor to apply.
      */
      virtual void Apply(CanvasNodeVisitor& visitor) const = 0;

    protected:

      //! Constructs an empty CanvasNode.
      CanvasNode();

      //! Copies a CanvasNode.
      /*!
        \param node The CanvasNode to copy.
      */
      CanvasNode(const CanvasNode& node);

      //! Clones this CanvasNode.
      virtual std::unique_ptr<CanvasNode> Clone() const = 0;

      //! Sets the type.
      /*!
        \param type The type.
      */
      void SetType(const CanvasType& type);

      //! Sets the text.
      /*!
        \param text The text.
      */
      void SetText(std::string text);

      //! Adds a child.
      /*!
        \param name The name of the child.
        \param node The CanvasNode to add as a child.
      */
      void AddChild(std::string name, std::unique_ptr<CanvasNode> node);

      //! Removes a child.
      /*!
        \param child The child to remove.
      */
      void RemoveChild(const CanvasNode& child);

      //! Sets a child node.
      /*!
        \param child The child to set.
        \param replacement The replacement child.
      */
      void SetChild(const CanvasNode& child, std::unique_ptr<CanvasNode> node);

      //! Renames a child node.
      /*!
        \param child The child to set.
        \param name The child's new name.
      */
      void RenameChild(const CanvasNode& child, std::string name);

      //! Sets whether this node is visible.
      /*!
        \param isVisible <code>true</code> to set this node as visible.
      */
      void InternalSetVisible(bool isVisible);

      //! Sets whether this node is read-only.
      /*!
        \param isReadOnly <code>true</code> to set this node as read-only.
      */
      void InternalSetReadOnly(bool isReadOnly);

      //! Returns a reset instance of this CanvasNode.
      virtual std::unique_ptr<CanvasNode> Reset() const;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct Beam::DataShuttle;
      friend class CanvasNodeBuilder;
      CanvasNode* m_parent;
      std::vector<std::unique_ptr<CanvasNode>> m_children;
      std::shared_ptr<CanvasType> m_type;
      std::string m_text;
      std::string m_name;
      bool m_isVisible;
      bool m_isReadOnly;
      std::unordered_map<std::string, MetaData> m_metaData;
  };

  template<typename T>
  std::unique_ptr<T> CanvasNode::Clone(const T& node) {
    return Beam::static_pointer_cast<T>(
      static_cast<const CanvasNode&>(node).Clone());
  }

  template<Beam::IsShuttle S>
  void CanvasNode::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("children", m_children);
    if(Beam::IsReceiver<S>) {
      for(auto& child : m_children) {
        child->m_parent = this;
      }
    }
    shuttle.shuttle("type", m_type);
    shuttle.shuttle("text", m_text);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("is_visible", m_isVisible);
    shuttle.shuttle("is_read_only", m_isReadOnly);
    shuttle.shuttle("meta_data", m_metaData);
  }
}

#endif
