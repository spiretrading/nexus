#ifndef SPIRE_TASK_KEY_BINDINGS_TABLE_MODEL_HPP
#define SPIRE_TASK_KEY_BINDINGS_TABLE_MODEL_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsTableModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {
  
  //! Represents a model for displaying order key bindings.
  class TaskKeyBindingsTableModel : public KeyBindingsTableModel {
    public:

      //! The columns represented by the model.
      enum class Columns {

        //! The key binding name column.
        NAME,

        //! The security column.
        SECURITY,

        //! The destination column.
        DESTINATION,

        //! The order type column.
        ORDER_TYPE,

        //! The order side column.
        SIDE,

        //! The quantity column.
        QUANTITY,

        //! The time in force column.
        TIME_IN_FORCE,

        //! The custom tags column.
        CUSTOM_TAGS,

        //! The key binding column.
        KEY_BINDING
      };

      //! Signals that an item's data was modified.
      /*
        \param index The index that was modified.
      */
      using ItemModifiedSignal = Signal<void (const QModelIndex& index)>;

      //! Specifies the action type used by the model.
      using Action = KeyBindings::OrderActionBinding;

      //! Constructs a TaskKeyBindingsTableModel.
      /*
        \param bindings The initial key bindings.
        \param parent The parent object.
      */
      explicit TaskKeyBindingsTableModel(std::vector<Action> bindings,
        QObject* parent = nullptr);

      //! Updates the key bindings to display.
      /*
        \param bindings The updated key bindings.
      */
      void set_key_bindings(const std::vector<Action>& bindings);

      //! Connects a slot to the item modified signal.
      boost::signals2::connection connect_item_modified_signal(
        const ItemModifiedSignal::slot_type& slot) const;

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      Qt::ItemFlags flags(const QModelIndex& index) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

      bool removeRows(int row, int count,
        const QModelIndex& parent = QModelIndex()) override;

      bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;

    private:
      mutable ItemModifiedSignal m_modified_signal;
      std::vector<Action> m_key_bindings;

      void insert_row_if_empty(const QModelIndex& index);
      bool is_row_empty(int row) const;
  };
}

#endif
