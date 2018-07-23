#ifndef SPIRE_LUASCRIPTPARAMETERSTABLE_HPP
#define SPIRE_LUASCRIPTPARAMETERSTABLE_HPP
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QTableWidget>
#include "Spire/CanvasView/CanvasView.hpp"

class QComboBox;

namespace Spire {

  /*! \class LuaScriptParametersTable
      \brief Displays a table listing a LuaScript's parameters.
   */
  class LuaScriptParametersTable : public QTableWidget {
    public:

      //! Constructs a LuaScriptParametersTable.
      /*!
        \param dialog The LuaScriptDialog to display the table for.
      */
      LuaScriptParametersTable(QWidget* dialog);

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
      LuaScriptDialog* m_dialog;
      QComboBox* m_typeComboBox;
  };
}

#endif
