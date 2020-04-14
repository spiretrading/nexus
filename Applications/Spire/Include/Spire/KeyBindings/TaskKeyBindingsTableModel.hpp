#ifndef SPIRE_TASK_KEY_BINDINGS_TABLE_MODEL_HPP
#define SPIRE_TASK_KEY_BINDINGS_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {
  
  class TaskKeyBindingsTableModel : public QAbstractTableModel {
    public:

      //! Specifies the action type used by the model.
      using Action = KeyBindings::OrderActionBinding;

      explicit TaskKeyBindingsTableModel(std::vector<Action> bindings,
        QObject* parent = nullptr);

      //! Updates the key bindings to display.
      /*
        \param bindings The updated key bindings.
      */
      void set_key_bindings(const std::vector<Action>& bindings);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      Qt::ItemFlags flags(const QModelIndex& index) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

      bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    private:
      std::vector<Action> m_key_bindings;
      CustomVariantItemDelegate* m_item_delegate;
  };
}

#endif
