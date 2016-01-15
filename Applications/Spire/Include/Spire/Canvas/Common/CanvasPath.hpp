#ifndef SPIRE_CANVASPATH_HPP
#define SPIRE_CANVASPATH_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class CanvasPath
      \brief Represents a path from a root CanvasNode to a child CanvasNode.
   */
  class CanvasPath {
    public:

      //! Constructs a CanvasPath.
      /*!
        \param fullPath The full path from the root to the child.
      */
      CanvasPath(const char* fullPath);

      //! Constructs a CanvasPath.
      /*!
        \param fullPath The full path from the root to the child.
      */
      CanvasPath(std::string fullPath);

      //! Constructs a CanvasPath.
      /*!
        \param node The CanvasNode to get the path to.
      */
      CanvasPath(const CanvasNode& node);

      //! Returns the full path from the root to the child.
      const std::string& GetPath() const;

    private:
      std::string m_path;
  };

  //! Returns the CanvasNode represented by a path.
  /*!
    \param path The path of the CanvasNode to return.
    \param root The root CanvasNode.
    \return The CanvasNode within the <i>root</i> to the <i>path</i>.
  */
  const CanvasNode& GetNode(const CanvasPath& path, const CanvasNode& root);

  //! Returns the CanvasNode represented by a path.
  /*!
    \param path The path of the CanvasNode to return.
    \param root The root CanvasNode.
    \return The CanvasNode within the <i>root</i> to the <i>path</i>.
  */
  boost::optional<const CanvasNode&> FindNode(const CanvasPath& path,
    const CanvasNode& root);
}

#endif
