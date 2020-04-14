#ifndef SPIRE_INPUT_FIELD_ITEM_DELEGATE_HPP
#define SPIRE_INPUT_FIELD_ITEM_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Spire/KeyBindings/InputFieldEditor.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class InputFieldItemDelegate : public QStyledItemDelegate {
    public:

      using ItemModifiedSignal = Signal<void (const QModelIndex& index)>;

      explicit InputFieldItemDelegate(std::vector<QString> items,
        QWidget* parent = nullptr);

      boost::signals2::connection connect_item_modified_signal(
        const ItemModifiedSignal::slot_type& slot) const;

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
    
      QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    private:
      mutable ItemModifiedSignal m_item_modified_signal;
      std::vector<QString> m_items;

      void on_editing_finished();
  };
}

#endif
