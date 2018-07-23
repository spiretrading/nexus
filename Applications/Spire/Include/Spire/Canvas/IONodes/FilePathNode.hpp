#ifndef SPIRE_FILEPATHNODE_HPP
#define SPIRE_FILEPATHNODE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /*! \class FilePathNode
      \brief A CanvasNode that allows the user to select a file path.
   */
  class FilePathNode : public CanvasNode {
    public:

      //! Constructs a FilePathNode.
      FilePathNode();

      //! Constructs a FilePathNode.
      /*!
        \param path The initial path.
      */
      FilePathNode(std::string path);

      //! Returns the path represented.
      const std::string& GetPath() const;

      //! Sets the path referred to by this instance.
      /*!
        \param path The path to refer to.
        \return A clone of this instance referring to the specified <i>path</i>.
      */
      std::unique_ptr<FilePathNode> SetPath(std::string path) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_path;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void FilePathNode::Shuttle(Shuttler& shuttle, unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("path", m_path);
  }
}

#endif
