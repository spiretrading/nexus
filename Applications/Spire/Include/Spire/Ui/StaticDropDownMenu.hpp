#ifndef SPIRE_STATIC_DROP_DOWN_MENU_HPP
#define SPIRE_STATIC_DROP_DOWN_MENU_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"

namespace Spire {

  class StaticDropDownMenu : public QWidget {
    public:

      using IndexSelectedSignal = Signal<void (int index)>;

      using ValueSelectedSignal = Signal<void (const QVariant& value)>;

      explicit StaticDropDownMenu(const std::vector<QVariant>& items,
        QWidget* parent = nullptr);

      virtual ~StaticDropDownMenu() = default;

      StaticDropDownMenu(const std::vector<QVariant>& items,
        const QString& display_text, QWidget* parent = nullptr);

      int item_count() const;

      void insert_item(DropDownItem* item);

      virtual void remove_item(int index);

      void set_items(const std::vector<QVariant>& items);

      const QVariant& get_current_item() const;

      boost::signals2::connection connect_index_selected_signal(
        const IndexSelectedSignal::slot_type& slot) const;

      boost::signals2::connection connect_value_selected_signal(
        const ValueSelectedSignal::slot_type& slot) const;

    protected:
      void paintEvent(QPaintEvent* event);
      void resizeEvent(QResizeEvent* event);

    private:
      mutable ValueSelectedSignal m_value_selected_signal;
      QVariant m_current_item;
      QString m_display_text;
      QImage m_dropdown_image;
      DropDownList* m_menu_list;
      CustomVariantItemDelegate m_item_delegate;
      boost::signals2::scoped_connection m_menu_selection_connection;

      void draw_item_text(const QString& text, QPainter& painter);
      void on_item_selected(const QVariant& value);
  };
}

#endif
