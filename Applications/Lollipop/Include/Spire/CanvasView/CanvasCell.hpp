#ifndef SPIRE_CANVASCELL_HPP
#define SPIRE_CANVASCELL_HPP
#include <QTableWidget>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class CanvasCell
      \brief Interface for a CanvasNode's representation in a QTableWidget.
   */
  class CanvasCell : public QTableWidgetItem {
    public:
      virtual ~CanvasCell() = default;

      //! Returns the CanvasNode represented by this cell.
      virtual const CanvasNode& GetNode() const = 0;

    protected:

      //! Constructs a CanvasCell.
      CanvasCell() = default;
  };
}

#endif
