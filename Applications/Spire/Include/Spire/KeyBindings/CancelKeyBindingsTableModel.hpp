#ifndef SPIRE_CANCEL_KEY_BINDINGS_TABLE_MODEL_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  //! Represents a model for displaying cancel action key bindings.
  class CancelKeyBindingsTableModel : public QAbstractTableModel {
    public:

      //! Specifies the action type used by the model.
      using Action = KeyBindings::CancelAction;

      //! Specifies the binding type used by the model.
      using Binding = KeyBindings::CancelActionBinding;

      //! Returns the cancel action associated with the given row.
      /*
        \param row The row to get the associated cancel action from.
      */
      static Action get_cancel_action(int row);

      //! Constructs a CancelKeyBindingsTableModel with initial key
      //! bindings.
      /*
        \param bindings The initial key bindings to display.
        \param parent The parent object.
      */
      explicit CancelKeyBindingsTableModel(std::vector<Binding> bindings,
        QObject* parent = nullptr);

      //! Updates the key bindings to display.
      /*
        \param bindings The updated key bindings.
      */
      void set_key_bindings(const std::vector<Binding>& bindings);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      Qt::ItemFlags flags(const QModelIndex& index) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

      bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    private:
      std::vector<Binding> m_key_bindings;
  };
}

#endif
