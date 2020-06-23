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

      //! Signal type for recent color updates.
      /*!
        \param recent_colors The updated recent colors.
      */
      using RecentColorsSignal =
        Signal<void (const RecentColors& recent_colors)>;

      //! Constructs a ColorSelectorButton.
      /*!
        \param current_color The current color to display in the button.
        \param recent_colors The recent colors to display in the drop down.
        \param parent The parent widget.
      */
      ColorSelectorButton(const QColor& current_color,
        const RecentColors& recent_colors, QWidget* parent = nullptr);

      //! Sets the button's color.
      /*!
        \param color The color to set.
      */
      void set_color(const QColor& color);

      //! Sets the recent colors for the button's drop down.
      /*!
        \param recent_colors The recent colors to set.
      */
      void set_recent_colors(const RecentColors& recent_colors);

      //! Connects a slot to the color selection signal.
      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

      //! Connects a slot to the recent colors signal.
      boost::signals2::connection connect_recent_colors_signal(
        const RecentColorsSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      mutable RecentColorsSignal m_recent_colors_signal;
      QColor m_current_color;
      RecentColors m_recent_colors;
      ColorSelectorDropDown* m_dropdown;

      void hide_dropdown();
      void move_color_dropdown();
      void on_button_clicked();
      void on_color_selected(const QColor& color);
  };
}

#endif
