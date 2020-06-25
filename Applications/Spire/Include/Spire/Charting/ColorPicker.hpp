#ifndef SPIRE_COLOR_PICKER_HPP
#define SPIRE_COLOR_PICKER_HPP
#include <QLabel>
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  //! Displays a color picker and signals when a color is selected.
  class ColorPicker : public QWidget {
    public:

      //! Signals that a color has been previewed or selected.
      /*!
        \param color The color that has been previewed or selected.
      */
      using ColorSignal = Signal<void (const QColor& color)>;

      //! Constructs a ColorPicker.
      /*!
        \param width The width of the color picker image.
        \param height The height of the color picker image.
        \param parent The parent widget.
      */
      ColorPicker(int width, int height, QWidget* parent = nullptr);

      //! Calls the provided slot when the color preview signal is triggered.
      /*!
        \param slot The slot to call.
      */
      boost::signals2::connection connect_preview_signal(
        const ColorSignal::slot_type& slot) const;

      //! Calls the provided slot when the color selected signal is triggered.
      /*!
        \param slot The slot to call.
      */
      boost::signals2::connection connect_selected_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable ColorSignal m_preview_signal;
      mutable ColorSignal m_selected_signal;
      QLabel* m_gradient_label;

      QColor gradient_color_at(const QPoint& pos);
  };
}

#endif
