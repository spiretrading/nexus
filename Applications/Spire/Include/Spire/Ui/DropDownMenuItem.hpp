#ifndef SPIRE_DROP_DOWN_MENU_ITEM_HPP
#define SPIRE_DROP_DOWN_MENU_ITEM_HPP
#include <QImage>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Represents an item used in a DropDownMenuList.
  class DropDownMenuItem : public QWidget {
    public:

      //! Signals that the item was highlighted with the mouse.
      /*!
        \param value The value of the DropDownMenuItem.
      */
      using HighlightedSignal = Signal<void (QVariant value)>;

      //! Signals that the item was selected.
      /*!
        \param value The value of the DropDownMenuItem.
      */
      using SelectedSignal = Signal<void (QVariant value)>;

      //! Constructs a DropDownMenuItem.
      /*!
        \param value The value represented by the DropDownMenuItem.
        \param parent The parent widget.
      */
      explicit DropDownMenuItem(QVariant value, QWidget* parent = nullptr);

      //! Consstructs a DropDownMenuItem with an icon.
      /*!
        \param value The value represented by the DropDownMenuItem.
        \param icon The icon to display beside the value.
        \param parent The parent widget.
      */
      DropDownMenuItem(QVariant value, QImage icon, QWidget* parent = nullptr);

      //! Returns the item's value.
      const QVariant& get_value() const;

      //! Returns true iff the item is highlighted.
      bool is_highlighted() const;

      //! Displays the item's highlight style.
      void set_highlight();

      //! Removes the item's highlight style.
      void reset_highlight();

      //! Connects a slot to the highlighted signal.
      boost::signals2::connection connect_highlighted_signal(
        const HighlightedSignal::slot_type& slot) const;
      
      //! Connects a slot to the selected signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void enterEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QVariant m_value;
      QImage m_icon;
      mutable HighlightedSignal m_highlighted_signal;
      mutable SelectedSignal m_selected_signal;
      bool m_is_highlighted;
      CustomVariantItemDelegate m_item_delegate;
  };
}

#endif
