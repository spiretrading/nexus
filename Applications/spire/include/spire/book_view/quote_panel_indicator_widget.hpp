#ifndef SPIRE_QUOTE_PANEL_INDICATOR_WIDGET_HPP
#define SPIRE_QUOTE_PANEL_INDICATOR_WIDGET_HPP
#include <QTimer>
#include <QWidget>

namespace Spire {

  //! Displays a widget with an animated background color.
  class QuotePanelIndicatorWidget : public QWidget {
    public:

      //! Constructs a QuotePanelIndicatorWidget with a default background
      //! color of black.
      QuotePanelIndicatorWidget(QWidget* parent = nullptr);

      //! Sets the background color.
      /*
        \param color The background color.
      */
      void set_color(const QColor& color);

      //! Sets the background color with partial opacity and animates it
      //! to full opacity.
      /*
        \param color The background color.
      */
      void animate_color(const QColor& color);

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QColor m_color;
      QTimer m_animation_timer;
  };
}

#endif
