#ifndef SPIRE_OVERLAY_PANEL_HPP
#define SPIRE_OVERLAY_PANEL_HPP
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** 
   * Represents a overlay panel component which is used as a pop-up window
   * displaying a sub-component within it.
   */
  class OverlayPanel : public QWidget {
    public:

      /** Specifies the positioning of the panel relative to the parent. */
      enum class Positioning {

        /** The panel is not positioned relative to any other component. */
        NONE,

        /** The panel is positioned relative to the parent. */
        PARENT
      };

      /** Signals that the panel is closed. */
      using ClosedSignal = Signal<void ()>;

      /**
       * Constructs an OverlayPanel.
       * @param body The widget displayed in the OverlayPanel.
       * @param parent The parent widget that shows the OverlayPanel.
       */
      OverlayPanel(QWidget* body, QWidget* parent);

      /** Returns the body component displayed in the panel. */
      const QWidget& get_body() const;

      /** Returns the body component displayed in the panel. */
      QWidget& get_body();

      /**
       * Returns <code>true</code> when the panel can be closed on blur event.
       */
      bool is_closed_on_blur() const;

      /** Sets whether the panel closes on blur event.
       * @param is_closed_on_blur Sets the panel to be colsed on blur event iff
       *                          is_closed_on_blur is true.
       */
      void set_closed_on_blur(bool is_closed_on_blur);

      /** Returns the positioning of the panel relative to the parent. */
      Positioning get_positioning() const;

      /** Sets the positioning of the panel relative to the parent.
       * @param positioning The positioning of the panel relative to the parent.
       */
      void set_positioning(Positioning positioning);

      /** Connects a slot to the close signal. */
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable ClosedSignal m_closed_signal;
      QWidget* m_body;
      QWidget* m_top_level_window;
      bool m_is_closed_on_blur;
      bool m_is_closed;
      Positioning m_positioning;
      QPoint m_mouse_press_position;

      QScreen* get_current_screen(const QPoint& point) const;
      void fade(bool reverse);
      void position();
  };
}

#endif
