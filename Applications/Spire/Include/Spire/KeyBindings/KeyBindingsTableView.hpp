#ifndef SPIRE_KEY_BINDINGS_TABLE_VIEW_HPP
#define SPIRE_KEY_BINDINGS_TABLE_VIEW_HPP
#include <QHeaderView>
#include <QStyledItemDelegate>
#include "Spire/Ui/ScrollArea.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsTableModel.hpp"
#include "Spire/KeyBindings/CustomGridTableView.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  class KeyBindingsTableView : public ScrollArea {
    public:

      explicit KeyBindingsTableView(QHeaderView* header,
        QWidget* parent = nullptr);

      void set_column_delegate(int column, QStyledItemDelegate* delegate);

      void set_column_width(int column, int width);

      void set_model(QAbstractTableModel* model);

      void set_height(int height);

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
