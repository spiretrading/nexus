#ifndef SPIRE_TOOLIP_HPP
#define SPIRE_TOOLIP_HPP
#include <QTimer>
#include <QWidget>

namespace Spire {

  class Tooltip : public QWidget {
    public:
  
      //! Constructs a Tooltip.
      /*!
        \param body The widget displayed in the Tooltip.
        \param parent The parent widget that when hovered will show the
                      Tooltip.
      */
      Tooltip(QWidget* body, QWidget* parent);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      enum class Orientation {
        BOTTOM,
        TOP
      };

      QWidget* m_body;
      QPoint m_position_offset;
      QTimer m_show_timer;

      QPainterPath get_arrow_path() const;
      QMargins get_margins() const;
      Orientation get_orientation() const;
      QPoint get_position() const;
      void on_show_timeout();
  };

  //! Constructs a Tooltip with a text label.
  /*!
    \param label The text label.
    \param parent The Tooltip's parent widget.
  */
  Tooltip* make_text_tooltip(const QString& label, QWidget* parent);
}

#endif
