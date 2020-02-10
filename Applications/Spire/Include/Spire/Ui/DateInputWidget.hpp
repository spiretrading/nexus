#ifndef SPIRE_DATE_INPUT_WIDGET_HPP
#define SPIRE_DATE_INPUT_WIDGET_HPP
#include <QWidget>
#include <QHBoxLayout>
#include <QCalendarWidget>
#include <QLocale>
#include <QTableView>
#include <QHeaderView>

namespace Spire {

  class DateInputWidget : public QWidget {
    public:

      DateInputWidget(QWidget* parent = nullptr);
  };
}

#endif
