#ifndef SPIRE_LINKEDNODE_HPP
#define SPIRE_LINKEDNODE_HPP
#include <memory>
#include <string>
#include <Beam/Utilities/Casts.hpp>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class LinkedNode
      \brief An interface to a CanvasNode that is linked to another CanvasNode.
   */
  class LinkedNode {
    public:

      //! Returns a clone with a specified referent.
      /*!
        \param node The CanvasNode to set the referent of.
        \param referent The referent to set.
      */
      template<typename T>
      static std::unique_ptr<T> SetReferent(const T& node,
        const std::string& referent);

      virtual ~LinkedNode() = default;

      //! Returns the referent.
      virtual const std::string& GetReferent() const = 0;

      //! Returns a clone with a specified referent.
      /*!
        \param referent The referent to set.
      */
      virtual std::unique_ptr<CanvasNode> SetReferent(
        const std::string& referent) const = 0;

      //! Finds the CanvasNode being referred to.
      virtual boost::optional<const CanvasNode&> FindReferent() const;
  };

  //! Computes the 'height' of the referent in a LinkedNode.
  /*!
    \param node The LinkedNode to compute the height of.
    \return The height of the referent from the <i>node</i>.
  */
  int GetReferentHeight(const LinkedNode& node);

  //! Relinks all CanvasNodes.
  /*!
    \param root The root CanvasNode to relink.
    \return A CanvasNode whose links have been relinked.
  */
  std::unique_ptr<CanvasNode> Relink(const CanvasNode& root);

  template<typename T>
  std::unique_ptr<T> LinkedNode::SetReferent(const T& node,
      const std::string& referent) {
    return Beam::static_pointer_cast<T>(node.SetReferent(referent));
  }
}

#endif
