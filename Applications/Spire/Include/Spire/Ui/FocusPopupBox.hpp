#ifndef SPIRE_FOCUS_POPUP_BOX_HPP
#define SPIRE_FOCUS_POPUP_BOX_HPP
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/GlobalPositionObserver.hpp"

namespace Spire {

  /**
   * Takes a body and allows the body to expand beyond the size of its parent
   * by popping out of its parent.
   */
  class FocusPopupBox : public QWidget {
    public:

      /**
       * Constructs a AlignableWrapper.
       * @param body The component to which the functionality is added.
       * @param parent The parent widget.
       */
      FocusPopupBox(QWidget& body, QWidget* parent = nullptr);

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
      FocusObserver m_focus_observer;
      FocusObserver m_body_focus_observer;
      GlobalPositionObserver m_position_observer;
      Alignment m_alignment;
      Alignment m_last_alignment;
      QPoint m_position;
      int m_min_height;
      int m_max_height;
      int m_above_space;
      int m_below_space;
      boost::signals2::scoped_connection m_focus_connection;
      boost::signals2::scoped_connection m_body_focus_connection;
      boost::signals2::scoped_connection m_position_connection;

      void on_focus(FocusObserver::State state);
      void on_body_focus(FocusObserver::State state);
      void on_position(const QPoint& position);
      void align();
      void adjust_size();
      void set_position(const QPoint& pos);
      void update_space();
  };
  
}

#endif
