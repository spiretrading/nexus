#ifndef SPIRE_TREND_LINE_EDITOR_HPP
#define SPIRE_TREND_LINE_EDITOR_HPP
#include <QWidget>
#include "spire/charting/charting.hpp"

namespace Spire {

  //! Displays a widget for selecting color and line styles.
  class TrendLineEditor : public QWidget {
    public:

      //! Signals that the widget's selected color has changed.
      using ColorSignal = Signal<void ()>;

      //! Signals that the widget's selected line style has changed.
      using StyleSignal = Signal<void ()>;

      //! Constructs a TrendLineEditor.
      /*
        \param parent The parent to the TrendLineEditor.
      */
      TrendLineEditor(QWidget* parent = nullptr);

      //! Returns the selected color.
      const QColor& get_color() const;

      //! Sets the selected color.
      void set_color(const QColor& color);

      //! Returns the selected line style.
      Qt::PenStyle get_style() const;

      //! Sets the selected line style.
      void set_style(Qt::PenStyle style);

      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;
    
      boost::signals2::connection connect_style_signal(
        const StyleSignal::slot_type& slot) const;

    private:
      QColor m_color;
      Qt::PenStyle m_line_style;
  };
}

#endif
