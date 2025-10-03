#ifndef SPIRE_OVERLAY_PANEL_HPP
#define SPIRE_OVERLAY_PANEL_HPP
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/GlobalPositionObserver.hpp"
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

      /**
       * Constructs an OverlayPanel.
       * @param body The widget displayed in the OverlayPanel.
       * @param parent The parent widget that shows the OverlayPanel.
       */
      OverlayPanel(QWidget& body, QWidget& parent);

      /** Returns the body component displayed in the panel. */
      const QWidget& get_body() const;

      /** Returns the body component displayed in the panel. */
      QWidget& get_body();

      /**
       * Returns <code>true</code> when the panel can be closed on a focus out
       * event.
       */
      bool is_closed_on_focus_out() const;

      /**
       * Sets whether the panel closes on focus out event.
       * @param is_closed_on_focus_out Sets the panel to be closed on a focus
                                       out event iff is_closed_on_focus_out is
                                       <code>true</code>.
       */
      void set_closed_on_focus_out(bool is_closed_on_focus_out);

      /** Returns <code>true</code> if the panel can be dragged. */
      bool is_draggable() const;

      /**
       * Sets whether the panel can be dragged.
       * @param is_draggable Sets the panel to be draggable iff is_draggable is
                             <code>true</code>.
       */
      void set_is_draggable(bool is_draggable);

      /** Returns the positioning of the panel relative to the parent. */
      Positioning get_positioning() const;

      /**
       * Sets the positioning of the panel relative to the parent.
       * @param positioning The positioning of the panel relative to the parent.
       */
      void set_positioning(Positioning positioning);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void closeEvent(QCloseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_body;
      Box* m_box;
      bool m_is_closed_on_focus_out;
      bool m_is_draggable;
      bool m_was_activated;
      Positioning m_positioning;
      QPoint m_mouse_pressed_position;
      FocusObserver m_parent_focus_observer;
      boost::signals2::scoped_connection m_parent_focus_connection;
      FocusObserver m_focus_observer;
      boost::signals2::scoped_connection m_focus_connection;
      GlobalPositionObserver m_parent_position_observer;
      boost::signals2::scoped_connection m_parent_position_connection;

      void position();
      void on_focus(FocusObserver::State state);
      void on_parent_focus(FocusObserver::State state);
      void on_parent_move(QPoint);
      void update_mask();
  };
}

#endif
