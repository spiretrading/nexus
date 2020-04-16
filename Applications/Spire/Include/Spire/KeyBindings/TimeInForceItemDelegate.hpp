#ifndef SPIRE_TIME_IN_FORCE_ITEM_DELEGATE_HPP
#define SPIRE_TIME_IN_FORCE_ITEM_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class TimeInForceItemDelegate : public QStyledItemDelegate {
    public:

      using ItemModifiedSignal = Signal<void (const QModelIndex& index)>;

      explicit TimeInForceItemDelegate(QWidget* parent = nullptr);

      boost::signals2::connection connect_item_modified_signal(
        const ItemModifiedSignal::slot_type& slot) const;

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;

    private:
      mutable ItemModifiedSignal m_item_modified_signal;

      void on_editing_finished();
  };
}

#endif
