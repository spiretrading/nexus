#ifndef SPIRE_KEY_BINDINGS_TABLE_VIEW_HPP
#define SPIRE_KEY_BINDINGS_TABLE_VIEW_HPP
#include <QHeaderView>
#include <QStyledItemDelegate>
#include "Spire/KeyBindings/CancelKeyBindingsTableModel.hpp"
#include "Spire/KeyBindings/CustomGridTableView.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/ScrollArea.hpp"

namespace Spire {

  //! Represents a table for displaying key bindings.
  class KeyBindingsTableView : public ScrollArea {
    public:

      //! Constructs a KeyBindingsTableView with a custom header.
      /*
        \param header The table's header.
        \param parent The parent widget.
      */
      explicit KeyBindingsTableView(QHeaderView* header,
        QWidget* parent = nullptr);

      //! Sets the delegate for the specified column.
      /*
        \param column The index of the column to apply the delegate to.
        \param delegate The delegate for the specified column.
      */
      void set_column_delegate(int column, QStyledItemDelegate* delegate);

      //! Sets the width of the specified column.
      /*
        \param column The index of the column to set the width of.
        \param width The width of the specified column.
      */
      void set_column_width(int column, int width);

      //! Sets the model associated with the table view.
      /*
        \param model The model to use for this view.
      */
      void set_model(QAbstractTableModel* model);

      //! Sets the table height.
      /*
        \param height The table height.
      */
      void set_height(int height);

      //! Sets the table width.
      /*
        \param width The table width.
      */
      void set_width(int width);

    private:
      CustomGridTableView* m_table;
      QHeaderView* m_header;

      void on_header_resize(int index, int old_size, int new_size);
      void on_header_move(int logical_index, int old_index, int new_index);
      void on_horizontal_slider_value_changed(int new_value);
  };
}

#endif
