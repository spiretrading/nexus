#ifndef SPIRE_FILEREADERNODESTRUCTURETABLE_HPP
#define SPIRE_FILEREADERNODESTRUCTURETABLE_HPP
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QTableWidget>
#include "Spire/CanvasView/CanvasView.hpp"

class QComboBox;

namespace Spire {

  /*! \class FileReaderNodeStructureTable
      \brief Displays a table showing a FileReaderNode's structure.
   */
  class FileReaderNodeStructureTable : public QTableWidget {
    public:

      //! Constructs a FileReaderNodeStructureTable.
      /*!
        \param dialog The FileReaderNodeDialog to display the table for.
      */
      FileReaderNodeStructureTable(QWidget* dialog);

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
      FileReaderNodeDialog* m_dialog;
      QComboBox* m_typeComboBox;
  };
}

#endif
