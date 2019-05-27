#ifndef SPIRE_COLOR_PICKER_HPP
#define SPIRE_COLOR_PICKER_HPP
#include <QImage>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a color picker and signals when a color is selected.
  class ColorPicker : public QWidget {
    public:

      using ColorSignal = Signal<void (const QColor& color)>;

      //! Constructs a ColorPicker.
      /*
        \param width The width of the color picker image.
        \param height The height of the color picker image.
        \param parent The parent widget.
      */
      ColorPicker(int width, int height, QWidget* parent = nullptr);

      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    private:
      mutable ColorSignal m_color_signal;
      QImage m_gradient;
  };
}

#endif
