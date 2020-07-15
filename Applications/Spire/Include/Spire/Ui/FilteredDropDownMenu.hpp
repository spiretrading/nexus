#ifndef SPIRE_FILTERED_DROP_DOWN_MENU_HPP
#define SPIRE_FILTERED_DROP_DOWN_MENU_HPP
#include <QLineEdit>
#include <QVariant>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"

namespace Spire {

  class FilteredDropDownMenu : public QLineEdit {
    public:

      using SelectedSignal = Signal<void (const QVariant& value)>;

      explicit FilteredDropDownMenu(const std::vector<QVariant>& items,
        QWidget* parent = nullptr);

      void set_items(const std::vector<QVariant>& items);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      DropDownList* m_menu_list;
      std::vector<QVariant> m_items;
      QVariant m_current_item;
      CustomVariantItemDelegate m_item_delegate;

      const std::vector<DropDownItem*> create_widget_items(
        const std::vector<QVariant>& items);
      const std::vector<DropDownItem*> create_widget_items(
        const std::vector<QVariant>& items, const QString& filter_text);
      void on_item_activated(const QVariant& item);
      void on_item_selected(const QVariant& item);
      void on_text_edited(const QString& text);
  };
}

#endif
