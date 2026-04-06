#ifndef SPIRE_POPUP_BOX_HPP
#define SPIRE_POPUP_BOX_HPP
#include <QWidget>
#include "Spire/Ui/GlobalPositionObserver.hpp"

namespace Spire {

  /**
   * Takes a body and allows the body to grow beyond the layout,
   * event expanding beyond the containing window when the content overflows.
   */
  class PopupBox : public QWidget {
    public:

      /**
       * Constructs a PopupBox.
       * @param body The component which can pop up.
       * @param parent The parent widget.
       */
      explicit PopupBox(QWidget& body, QWidget* parent = nullptr);

      /** Returns the body. */
      const QWidget& get_body() const;

      /** Returns the body. */
      QWidget& get_body();

      /** Returns the orientations that trigger popup. */
      Qt::Orientations get_overflow_directions() const;

      /**
       * Sets which directions trigger popup on overflow.
       * @param directions The orientations that trigger popup.
       */
      void set_overflow_directions(Qt::Orientations directions);

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void hideEvent(QHideEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_body;
      Qt::Orientations m_overflow_directions;
      QSize m_body_size;
      GlobalPositionObserver m_position_observer;

      bool is_popped_up() const;
      void popup();
      void restore();
      void handle_overflow();
      void on_position(const QPoint& position);
  };

  /**
   * Returns a PopupBox using the specified panel as its body,
   * applying a drop shadow when the body is displayed as a popup.
   * @param panel The widget used as the body of the PopupBox.
   * @param parent The parent widget.
   */
  PopupBox* make_popup_panel(QWidget& panel, QWidget* parent = nullptr);
}

#endif
