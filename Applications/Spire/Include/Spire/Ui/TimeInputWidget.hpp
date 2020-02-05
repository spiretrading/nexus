#ifndef SPIRE_TIME_INPUT_WIDGET_HPP
#define SPIRE_TIME_INPUT_WIDGET_HPP
#include <QTimeEdit>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class TimeInputWidget : public QWidget {
    public:

      TimeInputWidget(QWidget* parent = nullptr);

    private:
      QTimeEdit* m_time_edit;
      DropDownMenu* m_drop_down_menu;
  };
}

#endif
