#ifndef SPIRE_DROP_DOWN_COLOR_PICKER_HPP
#define SPIRE_DROP_DOWN_COLOR_PICKER_HPP
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Ui/DropShadow.hpp"

namespace Spire {

  //! Displays a button that activates a pop-up color picker.
  class DropDownColorPicker : public QWidget {
    public:

      //! Signals that a color has been selected.
      /*!
        \param color The color that was selected.
      */
      using ColorSignal = Signal<void (const QColor& color)>;

      //! Constructs a DropdownColorPicker.
      /*!
        \param parent The parent widget.
      */
      explicit DropDownColorPicker(QWidget* parent = nullptr);

      //! Calls the provided slot when the color selected signal is triggered.
      /*!
        \param slot The slot to call.
      */
      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      void enterEvent(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      ColorPicker* m_color_picker;
      QColor m_current_color;
      QColor m_stored_color;
      bool m_has_mouse;
      std::unique_ptr<DropShadow> m_color_picker_drop_shadow;

      void on_color_preview(const QColor& color);
      void on_color_selected(const QColor& color);
      void move_color_picker();
  };
}

#endif
