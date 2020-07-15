#ifndef SPIRE_STATIC_DROP_DOWN_MENU_HPP
#define SPIRE_STATIC_DROP_DOWN_MENU_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"

namespace Spire {

  class StaticDropDownMenu : public QWidget {
    public:

      using SelectedSignal = Signal<void (const QVariant& value)>;

      explicit StaticDropDownMenu(const std::vector<QVariant>& items,
        QWidget* parent = nullptr);

      StaticDropDownMenu(const std::vector<QVariant>& items,
        const QString& display_text, QWidget* parent = nullptr);

      // TODO: demo this, make DropDownList responsible for resizing itself,
      //       add parameter somewhere to make it an option to have fixed width
      //       dropdown windows vs. windows that resize to parent width.
      void set_list_width(int width);

      void set_current_item(const QVariant& item);

      void set_items(const std::vector<QVariant>& items);

      const QVariant& get_current_item() const;

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void paintEvent(QPaintEvent* event);
      void resizeEvent(QResizeEvent* event);

    private:
      mutable SelectedSignal m_selected_signal;
      QVariant m_current_item;
      QString m_display_text;
      QImage m_dropdown_image;
      DropDownList* m_menu_list;
      CustomVariantItemDelegate m_item_delegate;

      void draw_item_text(const QString& text, QPainter& painter);
      void on_item_selected(const QVariant& value);
  };
}

#endif
