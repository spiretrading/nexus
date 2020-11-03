#ifndef SPIRE_STATIC_DROP_DOWN_MENU_HPP
#define SPIRE_STATIC_DROP_DOWN_MENU_HPP
#include <QLineEdit>
#include <QTimer>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"

namespace Spire {

  //! Represents a DropDownMenu that isn't modified by user interactions.
  class StaticDropDownMenu : public QLineEdit {
    public:

      //! The styles available to render the StaticDropDownMenu.
      enum class Style {

        //! Render using the default style.
        DEFAULT,

        //! Render using the table cell style.
        CELL
      };

      //! Signals that an item was selected.
      /*!
        \param index The index of the selected item.
      */
      using IndexSelectedSignal = Signal<void (int index)>;

      //! Signals that an item was selected.
      /*!
        \param value The value of the selected item.
      */
      using ValueSelectedSignal = Signal<void (const QVariant& value)>;

      //! Constructs a StaticDropDownMenu.
      /*!
        \param items The items to display.
        \param parent The parent widget.
      */
      explicit StaticDropDownMenu(std::vector<QVariant> items,
        QWidget* parent = nullptr);

      //! Constructs a StaticDropDownMenu with a fixed input label.
      /*!
        \param items The items to display.
        \param display_text The fixed input label text to display.
        \param parent The parent widget.
      */
      StaticDropDownMenu(std::vector<QVariant> items,
        const QString& display_text, QWidget* parent = nullptr);

      //! Returns the number of items in the list.
      virtual int item_count() const;

      //! Appends an item to the list.
      virtual void insert_item(DropDownItem* item);

      //! Removes and deletes the item at the given index.
      /*!
        \param index The index of the item to remove.
      */
      virtual void remove_item(int index);

      //! Sets the items to display, overwriting any existing items.
      /*!
        \param items The items to display.
      */
      virtual void set_items(const std::vector<QVariant>& items);

      //! Returns the currently selected item.
      virtual QVariant get_current_item() const;

      //! Returns the previously activated item, or the currently selected item
      //! if there is no currently activated item.
      virtual QVariant get_last_item() const;

      //! Sets the current item iff the given item is currently in the menu.
      /*!
        \param item The current item.
      */
      virtual void set_current_item(const QVariant& item);

      //! True iff the next item should be activated when the list is shown
      //! using the down arrow key.
      virtual void set_next_activated(bool is_next_activated);

      //! Sets the StaticDropDownMenu's style.
      /*!
        \param style The menu's style.
      */
      void set_style(Style style);

      //! Connects a slot to the index selected signal.
      virtual boost::signals2::connection connect_index_selected_signal(
        const IndexSelectedSignal::slot_type& slot) const;

      //! Connects a slot to the value selected signal.
      virtual boost::signals2::connection connect_value_selected_signal(
        const ValueSelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable ValueSelectedSignal m_value_selected_signal;
      QVariant m_current_item;
      QString m_display_text;
      QVariant m_last_activated_item;
      Style m_style;
      QImage m_dropdown_image;
      QImage m_disabled_dropdown_image;
      DropDownList* m_menu_list;
      CustomVariantItemDelegate m_item_delegate;
      boost::signals2::scoped_connection m_menu_selection_connection;
      boost::signals2::scoped_connection m_menu_activated_connection;
      QString m_entered_text;
      QTimer m_input_timer;
      bool m_is_next_activated;

      void draw_arrow(const QImage& arrow_image, QPainter& painter);
      void draw_background(const QColor& color, QPainter& painter);
      void draw_border(const QColor& color, QPainter& painter);
      void draw_item_text(const QString& text, QPainter& painter);
      void on_input_timeout();
      void on_item_activated(const QVariant& value);
      void on_item_selected(const QVariant& value);
      void on_key_press(QKeyEvent* event);
  };
}

#endif
