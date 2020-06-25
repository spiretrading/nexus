#ifndef SPIRE_TREND_LINE_EDITOR_HPP
#define SPIRE_TREND_LINE_EDITOR_HPP
#include <QWidget>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Spire/Ui/RecentColors.hpp"

namespace Spire {

  //! Displays a widget for selecting color and line styles.
  class TrendLineEditor : public QWidget {
    public:

      //! Signals that a color has been selected.
      using ColorSignal = Signal<void ()>;

      //! Signals that a line style has been selected.
      using StyleSignal = Signal<void ()>;

      //! Constructs a TrendLineEditor.
      /*!
        \param parent The parent to the TrendLineEditor, and reference for
                      determining the editor's position.
      */
      explicit TrendLineEditor(QWidget* parent = nullptr);

      //! Returns the selected color.
      const QColor& get_color() const;

      //! Sets the selected color.
      /*!
        \param color The color to assign to the TrendLineEditor.
      */
      void set_color(const QColor& color);

      //! Returns the selected line style.
      TrendLineStyle get_style() const;

      //! Sets the selected line style.
      /*!
        \param style The style to assign to the TrendLineEditor.
      */
      void set_style(TrendLineStyle style);

      //! Calls the provided slot when the color signal is triggered.
      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;
    
      //! Calls the provided slot when the style signal is triggered.
      boost::signals2::connection connect_style_signal(
        const StyleSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      mutable StyleSignal m_style_signal;
      QColor m_color;
      TrendLineStyle m_line_style;

      void on_color_change(const QColor& color);
      void on_style_change(TrendLineStyle style);
      void move_to_parent();
  };
}

#endif
