#ifndef SPIRE_TOOLIP_HPP
#define SPIRE_TOOLIP_HPP
#include <QTimer>
#include <QWidget>

namespace Spire {

  class Tooltip : public QWidget {
    public:

      //! Represents standard options for the Tooltip's position.
      enum class Position {

        //! The Tooltip is positioned relative to the bottom left of the parent
        //! widget.
        BOTTOM_LEFT,
      
        //! The Tooltip is positioned relative to the mouse pointer.
        MOUSE,
      };
  
      //! Constructs a Tooltip with a default Position of BOTTOM_LEFT.
      /*!
        \param body The widget displayed in the Tooltip.
        \param parent The parent widget that when hovered will show the
                      Tooltip.
      */
      Tooltip(QWidget* body, QWidget* parent);
  
      //! Sets the tooltip's position.
      void set_position(Position position);
  
      //! Sets the tooltip's position plus a given offset.
      void set_position(Position position, int x, int y);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      Position m_position;
      QPoint m_position_offset;
      QTimer m_show_timer;

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
