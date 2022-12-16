#ifndef SPIRE_POPUP_BOX_HPP
#define SPIRE_POPUP_BOX_HPP
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/GlobalPositionObserver.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Takes a body and allows the body to expand beyond the size of its parent
   * by popping out of its parent.
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

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      enum class Alignment : std::uint8_t {
        NONE,
        ABOVE,
        BELOW
      };
      QWidget* m_body;
      QWidget* m_window;
      FocusObserver m_body_focus_observer;
      FocusObserver m_focus_observer;
      GlobalPositionObserver m_position_observer;
      Alignment m_alignment;
      QPoint m_position;
      QSize m_last_size;
      int m_min_height;
      int m_max_height;
      int m_above_space;
      int m_below_space;
      int m_right_space;
      boost::signals2::scoped_connection m_focus_connection;

      bool has_popped_up() const;
      void align();
      void adjust_size();
      void set_position(const QPoint& pos);
      void update_window();
      void update_space();
      void on_body_focus(FocusObserver::State state);
      void on_focus(FocusObserver::State state);
      void on_position(const QPoint& position);
  };
}

#endif
