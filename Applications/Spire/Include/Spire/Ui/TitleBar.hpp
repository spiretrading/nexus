#ifndef SPIRE_TITLE_BAR_HPP
#define SPIRE_TITLE_BAR_HPP
#include <QHBoxLayout>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a window's title bar. */
  class TitleBar : public QWidget {
    public:

      /**
       * Constructs a title bar.
       * @param icon The window icon to display at the top-left corner.
       * @param parent The parent widget.
       */
      explicit TitleBar(QImage icon, QWidget* parent = nullptr);

      /**
       * Sets the icon to display.
       * @param icon The icon to display when the window has focus.
       */
      void set_icon(const QImage& icon);

      /** Returns the title text label. */
      const TextBox& get_title_label() const;

    protected:
      void changeEvent(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QHBoxLayout* m_container_layout;
      Button* m_window_button;
      TextBox* m_title_label;
      Button* m_minimize_button;
      Button* m_maximize_button;
      Button* m_restore_button;
      Button* m_close_button;

      void connect_window_signals();
      void on_window_title_change(const QString& title);
      void on_minimize_button_press();
      void on_maximize_button_press();
      void on_restore_button_press();
      void on_close_button_press();
  };
}

#endif
