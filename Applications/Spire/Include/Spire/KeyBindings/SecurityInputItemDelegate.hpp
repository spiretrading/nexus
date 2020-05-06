#ifndef SPIRE_SECURITY_INPUT_ITEM_DELEGATE_HPP
#define SPIRE_SECURITY_INPUT_ITEM_DELEGATE_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents an item delegate for displaying and editing the security of a
  //! key binding.
  class SecurityInputItemDelegate : public KeyBindingItemDelegate {
    public:

      //! Constructs a SecurityInputItemDelegate.
      /*
        \param model The security input model.
        \param parent The parent widget.
      */
      explicit SecurityInputItemDelegate(Beam::Ref<SecurityInputModel> model,
        QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      SecurityInputModel* m_model;
      std::vector<QString> m_items;
  };
}

#endif
