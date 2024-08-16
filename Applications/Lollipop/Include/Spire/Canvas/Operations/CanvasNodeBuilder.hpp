#ifndef SPIRE_CANVASNODEBUILDER_HPP
#define SPIRE_CANVASNODEBUILDER_HPP
#include <memory>
#include <string>
#include <boost/noncopyable.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasPath.hpp"

namespace Spire {

  /*! \class CanvasNodeBuilder
      \brief Constructs a new CanvasNode from an existing CanvasNode.
   */
  class CanvasNodeBuilder : private boost::noncopyable {
    public:

      //! Constructs a CanvasNodeBuilder from an existing CanvasNode.
      /*!
        \param node The CanvasNode to build from.
      */
      CanvasNodeBuilder(const CanvasNode& node);

      //! Returns the CanvasNode.
      std::unique_ptr<CanvasNode> Make();

      //! Converts a CanvasNode to a specified type.
      /*!
        \param path The path to the CanvasNode to convert.
        \param type The CanvasType that the node is to evaluate to.
      */
      void Convert(const CanvasPath& path, const CanvasType& type);

      //! Forces a conversion of a CanvasNode to a specified type.
      /*!
        \param path The path to the CanvasNode to convert.
        \param type The CanvasType that the node is to evaluate to.
      */
      void ForceConvert(const CanvasPath& path, const CanvasType& type);

      //! Replaces one CanvasNode with another.
      /*!
        \param path The path to the CanvasNode to replaced.
        \param replacement The CanvasNode to use as the substitute.
      */
      void Replace(const CanvasPath& path,
        std::unique_ptr<CanvasNode> replacement);

      //! Sets whether a CanvasNode is visible.
      /*!
        \param path The path to the CanvasNode to modify.
        \param visible Whether the <i>node</i> is visible.
      */
      void SetVisible(const CanvasPath& path, bool visible);

      //! Sets whether a CanvasNode is read-only.
      /*!
        \param path The path to the CanvasNode to modify.
        \param readOnly Whether the <i>node</i> is read-only.
      */
      void SetReadOnly(const CanvasPath& path, bool readOnly);

      //! Sets the meta-data of a specified CanvasNode.
      /*!
        \param path The path to the CanvasNode to modify.
        \param name The name of the meta-data to set.
        \param value The meta-data's value.
      */
      void SetMetaData(const CanvasPath& path, std::string name,
        CanvasNode::MetaData value);

      //! Deletes a CanvasNode's meta-data.
      /*!
        \param path The path to the CanvasNode to modify.
        \param name The name of the meta-data to delete.
      */
      void DeleteMetaData(const CanvasPath& path, const std::string& name);

      //! Resets a CanvasNode.
      /*!
        \param path The path to the CanvasNode to reset.
      */
      void Reset(const CanvasPath& node);

    private:
      std::unique_ptr<CanvasNode> m_node;

      void Commit(const CanvasNode& node,
        std::unique_ptr<CanvasNode> replacement);
  };
}

#endif
