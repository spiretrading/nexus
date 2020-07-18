#ifndef SPIRE_DROP_DOWN_MENU_HPP
#define SPIRE_DROP_DOWN_MENU_HPP
#include <QComboBox>
#include "Spire/Ui/DropDownItem.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a Spire-styled QComboBox.
  class DropDownMenu : public QWidget {
    public:

      //! Signals that an item was highlighted.
      /*!
        \param item The highlighted item.
      */
      using HighlightedSignal = Signal<void (const QString& item)>;

      //! Signals that the drop down menu was closed.
      using MenuClosedSignal = Signal<void ()>;

      //! Signals that an item was selected.
      /*!
        \param item The selected item.
      */
      using SelectedSignal = Signal<void (const QString& item)>;

      //! Constructs a DropDownMenu with the specified items. The first item
      //! in the vector is the initially selected item.
      /*!
        \param items The items to display in the menu.
        \param parent The parent widget to the DropDownMenu.
      */
      explicit DropDownMenu(const std::vector<QString>& items,
        QWidget* parent = nullptr);

      //! Sets the DropDownMenu's displayed text, without triggering the
      //! selected signal.
      /*
        \param text The text to display.
      */
      void set_current_text(const QString& text);

      //! Clears the items from the list and populates it with the specified
      //! items.
      /*!
        \param items The items to display in the menu.
      */
      void set_items(const std::vector<QString>& items);

      //! Returns the selected item.
      const QString& get_text() const;

      //! Connects a slot to the item highlight signal.
      boost::signals2::connection connect_highlighted_signal(
        const HighlightedSignal::slot_type& slot) const;

      //! Connects a slot to the menu closed signal.
      boost::signals2::connection connect_menu_closed_signal(
        const MenuClosedSignal::slot_type& slot) const;

      //! Connects a slot to the item selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable MenuClosedSignal m_menu_closed_signal;
      mutable SelectedSignal m_selected_signal;
      QString m_current_text;
      QImage m_dropdown_image;
      DropDownMenuList* m_menu_list;

      void hide_menu_list();
      void move_menu_list();
      void on_clicked();
      void on_item_selected(const QString& text);
  };
}

#endif
