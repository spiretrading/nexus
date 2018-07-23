#ifndef SPIRE_CANVASNODEOPERATIONS_HPP
#define SPIRE_CANVASNODEOPERATIONS_HPP
#include <memory>
#include <string>
#include <vector>
#include <Beam/Collections/View.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"

class QMimeData;

namespace Spire {

  //! Encodes a CanvasNode as MIME data.
  /*!
    \param nodes The list of CanvasNode's to encode.
    \return The MIME representation of the <i>nodes</i>.
  */
  std::unique_ptr<QMimeData> EncodeAsMimeData(
    const Beam::View<const CanvasNode>& nodes);

  //! Decodes a CanvasNode from MIME data.
  /*!
    \param data The MIME data to decode.
    \return The list of CanvasNodes represented by the MIME <i>data</i>.
  */
  std::vector<std::unique_ptr<CanvasNode>> DecodeFromMimeData(
    const QMimeData& data);

  //! Converts a CanvasNode to one compatible with a CanvasType.
  /*!
    \param node The CanvasNode to convert.
    \param type The CanvasType to convert the <i>node</i> to.
    \return A CanvasNode whose type has been converted.
  */
  std::unique_ptr<CanvasNode> Convert(std::unique_ptr<CanvasNode> node,
    const CanvasType& type);

  //! Forces the conversion of a CanvasNode.
  /*!
    \param node The CanvasNode to convert.
    \param type The CanvasType to convert the node to.
    \return The node converted to evaluate to the specified <i>type</i>,
            this will return the type's default CanvasNode if a normal
            conversion fails.
  */
  std::unique_ptr<CanvasNode> ForceConversion(std::unique_ptr<CanvasNode> node,
    const CanvasType& type);

  //! Returns <code>true</code> iff the clipboard is storing a CanvasNode.
  bool CheckClipboardForCanvasNode();

  /*! \struct CanvasNodeNameSuffix
      \brief Stores a CanvasNode name's top-level identifier and suffix.
   */
  struct CanvasNodeNameSuffix {

    //! The name's top-level identifier.
    std::string m_identifier;

    //! The name's suffix.
    std::string m_suffix;
  };

  //! Splits a qualified CanvasNode name.
  CanvasNodeNameSuffix SplitName(const std::string& name);

  //! Returns a CanvasNode's fully qualified name.
  std::string GetFullName(const CanvasNode& node);

  //! Returns the height from one CanvasNode to another.
  /*!
    \param from The CanvasNode to compute the height from.
    \param to The CanvasNode to compute the height to.
    \return The height between the two CanvasNodes, where the height is positive
            if <i>from</i> is a parent of <i>to</i>, otherwise the height is
            negative.
  */
  int GetHeight(const CanvasNode& from, const CanvasNode& to);

  //! Returns the path from one node to another.
  /*!
    \param source The CanvasNode where the path begins.
    \param destination The CanvasNode where the path ends.
    \return The path from the <i>source</i> to the <i>destination</i>.
  */
  std::string GetPath(const CanvasNode& source, const CanvasNode& destination);

  //! Appends two CanvasNode paths.
  /*!
    \param prefix The path's prefix.
    \param suffix The path's suffix.
    \return The <i>prefix</i> appended by the <i>suffix</i>.
  */
  std::string AppendCanvasNodePaths(const std::string& prefix,
    const std::string& suffix);

  //! Returns <code>true</code> iff two optional<CanvasNode>s are the same.
  bool IsSame(boost::optional<const CanvasNode&> a,
    boost::optional<const CanvasNode&> b);

  //! Returns <code>true</code> iff a <i>node</i> is the root.
  bool IsRoot(const CanvasNode& node);

  //! Returns the root CanvasNode.
  const CanvasNode& GetRoot(const CanvasNode& node);

  //! Returns the common ancestor of two CanvasNodes.
  /*!
    \param a One of the CanvasNodes.
    \param b A CanvasNode with the same root as <i>a</i>.
    \return The common ancestor of CanvasNodes <i>a</i> and <i>b</i>.
  */
  const CanvasNode& GetCommonAncestor(const CanvasNode& a, const CanvasNode& b);

  //! Tests if one CanvasNode is a parent of another.
  /*!
    \param a A CanvasNode to test.
    \param b A CanvasNode to test.
    \return <code>true</code> iff <i>a</i> is a parent of <i>b</i>.
  */
  bool IsParent(const CanvasNode& a, const CanvasNode& b);
}

#endif
