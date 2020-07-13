#ifndef SPIRE_STATIC_DROP_DOWN_MENU_HPP
#define SPIRE_STATIC_DROP_DOWN_MENU_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/DropDownList.hpp"

namespace Spire {

  class StaticDropDownMenu : public QWidget {
    public:

      using SelectedSignal = Signal<void (const QVariant& value)>;

      explicit StaticDropDownMenu(const std::vector<QString>& items,
        QWidget* parent = nullptr);

      // TODO: demo this, make DropDownList responsible for resizing itself,
      //       add parameter somewhere to make it an option to have fixed width
      //       dropdown windows vs. windows that resize to parent width.
      void set_list_width(int width);

      void set_current_text(const QString& text);

      void set_items(const std::vector<QString>& items);

      const QString& get_text() const;

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void paintEvent(QPaintEvent* event);
      void resizeEvent(QResizeEvent* event);

    private:
      mutable SelectedSignal m_selected_signal;
      QString m_current_text;
      QImage m_dropdown_image;
      DropDownList* m_menu_list;

      void on_item_selected(const QVariant& value);
  };
}

#endif
