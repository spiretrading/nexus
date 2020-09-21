#ifndef SPIRE_FILTERED_DROP_DOWN_MENU_HPP
#define SPIRE_FILTERED_DROP_DOWN_MENU_HPP
#include <QVariant>
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownMenuList.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

namespace Spire {

  //! Represents a drop down list that filters items that do not match the
  //! input text.
  class FilteredDropDownMenu : public TextInputWidget {
    public:

      //! Constructs a FilteredDropDownMenu.
      /*!
        \param items The initial items to display.
        \param parent The parent widget.
      */
      explicit FilteredDropDownMenu(std::vector<QVariant> items,
        QWidget* parent = nullptr);

      //! Returns the current item or an invalid QVariant if the delete key
      //! was the last key pressed.
      const QVariant& get_item_or_invalid() const;

      //! Returns the last valid item.
      const QVariant& get_item() const;

      //! Sets the current item iff it exists in the list of unfiltered items.
      /*!
        \param item The current item.
      */
      void set_current_item(const QVariant& item);

      //! Sets the items to display, overwriting any existing items.
      /*!
        \param items The new items to display.
      */
      void set_items(std::vector<QVariant> items);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      std::vector<QVariant> m_items;
      DropDownMenuList* m_menu_list;
      QVariant m_current_item;
      QVariant m_last_activated_item;
      bool m_was_click_focused;
      Qt::Key m_last_key;
      CustomVariantItemDelegate m_item_delegate;
      boost::signals2::scoped_connection m_item_activated_connection;
      boost::signals2::scoped_connection m_list_selection_connection;

      std::vector<DropDownMenuItem*> create_widget_items(
        const std::vector<QVariant>& items);
      std::vector<DropDownMenuItem*> create_widget_items(
        const std::vector<QVariant>& items, const QString& filter_text);
      void draw_border(const QColor& color);
      void draw_highlight(const QString& highlight_text);
      void on_editing_finished();
      void on_item_activated(const QVariant& item);
      void on_item_selected(const QVariant& item);
      void on_item_selected(const QVariant& item, bool emit_finished);
      void on_text_edited(const QString& text);
  };
}

#endif
