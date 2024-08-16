#ifndef SPIRE_BLOTTERTASKENTRYITEMDELEGATE_HPP
#define SPIRE_BLOTTERTASKENTRYITEMDELEGATE_HPP
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

namespace Spire {

  /*! \class BlotterTaskEntryItemDelegate
      \brief Displays custom editors/visuals for an TaskEntry.
   */
  class BlotterTaskEntryItemDelegate : public UI::CustomVariantItemDelegate {
    public:

      //! Constructs a BlotterTaskEntryItemDelegate.
      /*!
        \param userProfile The user's profile.
      */
      BlotterTaskEntryItemDelegate(Beam::Ref<UserProfile> userProfile);

      virtual ~BlotterTaskEntryItemDelegate();

      virtual QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option, const QModelIndex& index) const;

      virtual void setEditorData(QWidget* editor,
        const QModelIndex& index) const;

      virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const;

    private:
      void OnStickyToggled(bool checked);
  };
}

#endif
