#ifndef SPIRE_CUSTOMNODECHILDRENTABLE_HPP
#define SPIRE_CUSTOMNODECHILDRENTABLE_HPP
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QTableWidget>
#include "Spire/CanvasView/CanvasView.hpp"

class QComboBox;

namespace Spire {

  /*! \class CustomNodeChildrenTable
      \brief Displays a table listing a CustomNode's children.
   */
  class CustomNodeChildrenTable : public QTableWidget {
    public:

      //! Constructs a CustomNodeChildrenTable.
      /*!
        \param dialog The CustomNodeDialog to display the table for.
      */
      CustomNodeChildrenTable(QWidget* dialog);

      //! Returns the CanvasType combo box.
      QComboBox* GetCanvasTypeComboBox();

      //! Opens a CanvasType editor for an item.
      /*!
        \param item The item to open the CanvasType for.
      */
      void OpenTypeEditor(const QTableWidgetItem& item);

      //! Commits a CanvasType editor for an item.
      /*!
        \param item The item to commit the CanvasType to.
      */
      void CommitTypeEditor(QTableWidgetItem* item);

    protected:
      virtual void keyPressEvent(QKeyEvent* event);

      virtual void dropEvent(QDropEvent* event);

    private:
      CustomNodeDialog* m_dialog;
      QComboBox* m_typeComboBox;
  };
}

#endif
