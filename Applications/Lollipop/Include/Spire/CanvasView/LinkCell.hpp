#ifndef SPIRE_LINKCELL_HPP
#define SPIRE_LINKCELL_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QFlags>
#include "Spire/CanvasView/CanvasCell.hpp"

namespace Spire {

  /*! \class LinkCell
      \brief A cell that connects a parent CanvasNode to a single child.
   */
  class LinkCell : public CanvasCell {
    public:

      //! Constructs a LinkCell.
      /*!
        \param orientation The Orientation of this link.
        \param parent The CanvasNode represented by this cell.
        \param child The CanvasNode connected to the parent by this cell.
      */
      LinkCell(Qt::Orientation orientation,
        Beam::Ref<const CanvasNode> parent,
        Beam::Ref<const CanvasNode> child);

      //! Returns the orientation of this link.
      Qt::Orientation GetOrientation() const;

      //! Returns the child node.
      const CanvasNode& GetChild() const;

      virtual const CanvasNode& GetNode() const;

      virtual QTableWidgetItem* clone() const;

      virtual QVariant data(int role) const;

    private:
      Qt::Orientation m_orientation;
      const CanvasNode* m_parent;
      const CanvasNode* m_child;
  };
}

#endif
