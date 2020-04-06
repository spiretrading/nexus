#ifndef SPIRE_CANCEL_KEY_BINDINGS_TABLE_MODEL_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  //! Represents a model for displaying cancel action key bindings.
  class CancelKeyBindingsTableModel : public QAbstractTableModel {
    public:

      //! Constructs a CancelKeyBindingsTableModel with initial key
      //! bindings.
      /*
        \param bindings The initial key bindings to display.
        \param parent The parent object.
      */
      explicit CancelKeyBindingsTableModel(
        const std::vector<KeyBindings::CancelActionBinding>& bindings,
        QObject* parent = nullptr);

      //! Updates the key bindings to display.
      /*
        \param bindings The updated key bindings.
      */
      void set_key_bindings(
        const std::vector<KeyBindings::CancelActionBinding>& bindings);

      //! Returns the cancel action associated with the given row.
      /*
        \param row The row to get the associated cancel action from.
      */
      KeyBindings::CancelAction get_cancel_action(int row);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      Qt::ItemFlags flags(const QModelIndex &index) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

      bool setData(const QModelIndex &index, const QVariant &value,
        int role = Qt::EditRole) override;

    private:
      std::vector<KeyBindings::CancelActionBinding> m_key_bindings;
  };
}

#endif
