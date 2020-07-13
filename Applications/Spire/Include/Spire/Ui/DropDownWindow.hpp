#ifndef SPIRE_DROP_DOWN_WINDOW_HPP
#define SPIRE_DROP_DOWN_WINDOW_HPP
#include <QWidget>
#include "Spire/Ui/DropShadow.hpp"

namespace Spire {

  class DropDownWindow : public QWidget {
    public:

      explicit DropDownWindow(bool is_click_activated,
        QWidget* parent = nullptr);

      void set_widget(QWidget* widget);

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QWidget* m_widget;
      DropShadow* m_shadow;
      bool m_is_click_activated;

      void move_to_parent();
  };
}

#endif
