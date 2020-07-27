#ifndef SPIRE_STATIC_DROP_DOWN_MENU_HPP
#define SPIRE_STATIC_DROP_DOWN_MENU_HPP
#include <QTimer>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"

namespace Spire {

  //! Represents a DropDownMenu that isn't modified by user interactions.
  class StaticDropDownMenu : public QWidget {
    public:

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

      virtual ~StaticDropDownMenu() = default;

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
      virtual void set_items(std::vector<QVariant> items);

      //! Returns the currently selected item.
      virtual QVariant get_current_item() const;

      //! Connects a slot to the index selected signal.
      virtual boost::signals2::connection connect_index_selected_signal(
        const IndexSelectedSignal::slot_type& slot) const;

      //! Connects a slot to the value selected signal.
      virtual boost::signals2::connection connect_value_selected_signal(
        const ValueSelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event);
      void resizeEvent(QResizeEvent* event);

    private:
      mutable ValueSelectedSignal m_value_selected_signal;
      QVariant m_current_item;
      QString m_display_text;
      QVariant m_last_activated_item;
      QImage m_dropdown_image;
      QImage m_disabled_dropdown_image;
      DropDownList* m_menu_list;
      CustomVariantItemDelegate m_item_delegate;
      boost::signals2::scoped_connection m_menu_selection_connection;
      boost::signals2::scoped_connection m_menu_activated_connection;
      QString m_entered_text;
      QTimer m_input_timer;

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
