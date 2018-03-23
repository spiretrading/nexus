#ifndef SPIRE_TITLE_BAR_HPP
#define SPIRE_TITLE_BAR_HPP
#include <QLabel>
#include <QWidget>
#include "spire/ui/ui.hpp"

namespace spire {

  //! \brief Creates a title bar.
  class title_bar : public QWidget {
    public:
      title_bar(const QString& icon, const QString& unfocused_icon,
        QWidget* parent = nullptr);

    private:
      icon_button* m_icon;
      QLabel* m_title_label;
      icon_button* m_minimize_button;
      icon_button* m_maximize_button;
      icon_button* m_restore_button;
      icon_button* m_close_button;

      void on_window_title_change();
      void on_minimize_button_press();
      void on_maximize_button_press();
      void on_close_button_press();
  };
}

#endif
