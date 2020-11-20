#ifndef SPIRE_COLOR_SELECTOR_BUTTON_HPP
#define SPIRE_COLOR_SELECTOR_BUTTON_HPP
#include <QWidget>
#include "Spire/Ui/ColorSelectorDropDown.hpp"
#include "Spire/Ui/RecentColors.hpp"

namespace Spire {

  //! Represents a button with a drop down for selecting colors.
  class ColorSelectorButton : public QWidget {
    public:

      //! Signal type for color selection changes.
      /*!
        \param color The selected color.
      */
      using ColorSignal = Signal<void (const QColor& color)>;

      //! Constructs a ColorSelectorButton.
      /*!
        \param current_color The current color to display in the button.
        \param parent The parent widget.
      */
      ColorSelectorButton(const QColor& current_color,
        QWidget* parent = nullptr);

      //! Returns the current color.
      const QColor& get_color() const;

      //! Sets the button's color.
      /*!
        \param color The color to set.
      */
      void set_color(const QColor& color);

      //! Connects a slot to the color selection signal.
      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      QColor m_current_color;
      ColorSelectorDropDown* m_selector_widget;
      boost::signals2::scoped_connection m_color_connection;

      void on_color_selected(const QColor& color);
  };
}

#endif
