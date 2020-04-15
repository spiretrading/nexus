#ifndef SPIRE_SECURITY_INPUT_ITEM_DELEGATE_HPP
#define SPIRE_SECURITY_INPUT_ITEM_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Beam/Pointers/Ref.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class SecurityInputItemDelegate : public QStyledItemDelegate {
    public:

      using ItemModifiedSignal = Signal<void (const QModelIndex& index)>;

      explicit SecurityInputItemDelegate(Beam::Ref<SecurityInputModel> model,
        QWidget* parent = nullptr);

      boost::signals2::connection connect_item_modified_signal(
        const ItemModifiedSignal::slot_type& slot) const;

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;

    private:
      SecurityInputModel* m_model;
      mutable ItemModifiedSignal m_item_modified_signal;
      std::vector<QString> m_items;

      void on_editing_finished();
  };
}

#endif
