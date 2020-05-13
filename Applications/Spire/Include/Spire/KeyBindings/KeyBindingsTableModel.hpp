#ifndef SPIRE_KEY_BINDINGS_TABLE_MODEL_HPP
#define SPIRE_KEY_BINDINGS_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include <boost/optional.hpp>

namespace Spire {

  //! Represents a model for displaying key bindings with custom cell
  //! background colors.
  class KeyBindingsTableModel : public QAbstractTableModel {
    public:

      //! Constructs a KeyBindingsTableModel.
      /*
        \param parent The parent object.
      */
      explicit KeyBindingsTableModel(QObject* parent = nullptr);

      //! Sets the index of the focused cell.
      /*
        \param index The index of the focused cell.
      */
      void set_focus_highlight(const QModelIndex& index);

      //! Resets the focused cell index.
      void reset_focus_highlight();

      //! Sets the index of the hovered cell.
      /*
        \param index The hovered cell.
      */
      void set_hover_highlight(const QModelIndex& index);

      //! Resets the hovered cell index.
      void reset_hover_highlight();

      //! Sets the highlighted row.
      /*
        \param row The index of the highlighted row.
      */
      void set_row_highlight(int row);

      //! Resets the highlighted row.
      void reset_row_highlight();

      QVariant data(const QModelIndex& index, int role) const override;

    private:
      boost::optional<int> m_highlighted_row;
      QModelIndex m_focused_cell;
      QModelIndex m_hovered_cell;
  };
}

#endif
