#ifndef SPIRE_TREND_LINE_EDITOR_HPP
#define SPIRE_TREND_LINE_EDITOR_HPP
#include <QWidget>
#include "spire/charting/charting.hpp"
#include "spire/charting/trend_line.hpp"

namespace Spire {

  //! Displays a widget for selecting color and line styles.
  class TrendLineEditor : public QWidget {
    public:

      //! Constructs a TrendLineEditor.
      /*
        \param parent The parent to the TrendLineEditor, and reference for
                      determining the editor's position.
      */
      TrendLineEditor(QWidget* parent = nullptr);

      //! Returns the selected color.
      const QColor& get_color() const;

      //! Sets the selected color.
      void set_color(const QColor& color);

      //! Returns the selected line style.
      TrendLineStyle get_style() const;

      //! Sets the selected line style.
      void set_style(TrendLineStyle style);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      QColor m_color;
      TrendLineStyle m_line_style;

      void move_to_parent();
  };
}

#endif
