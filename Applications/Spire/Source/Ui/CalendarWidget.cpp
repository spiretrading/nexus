#include "Spire\Ui\CalendarWidget.hpp"
#include <QHeaderView>
#include <QLayout>
#include <QTableView>
#include <QTextCharFormat>
#include "Spire\Spire\Dimensions.hpp"

using namespace Spire;

CalendarWidget::CalendarWidget(QWidget* parent)
    : QCalendarWidget(parent) {
	setFixedSize(scale(168, 201));
	setFirstDayOfWeek(Qt::DayOfWeek::Sunday);
	setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
	auto inner_table = findChild<QTableView*>();
	setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
	inner_table->horizontalHeader()->setSectionResizeMode(
		QHeaderView::ResizeMode::Stretch);
	inner_table->horizontalHeader()->setDefaultSectionSize(scale_width(20));
  inner_table->horizontalHeader()->setMaximumSectionSize(scale_width(20));
	inner_table->horizontalHeader()->setMinimumHeight(scale_height(20));
	inner_table->horizontalHeader()->setMaximumHeight(scale_height(20));
	setStyleSheet(QString(R"(
      /* Applies to both dropdown and spinbox */
      /*QCalendarWidget QToolButton {
  	    height: 39px;
  	    width: 15px;
  	    color: white;
  	    font-size: 24px;
  	    icon-size: 56px, 56px;
      }*/

      QCalendarWidget QMenu {
  	    width: 150px;
  	    left: 20px;
  	    color: white;
  	    font-size: 18px;
  	    background-color: rgb(100, 100, 100);
      }

      QCalendarWidget QSpinBox { 
  	    width: 150px; 
  	    font-size: 24px; 
  	    color: white; 
  	    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333); 
  	    selection-background-color: rgb(136, 136, 136);
  	    selection-color: rgb(255, 255, 255);
      }

      QCalendarWidget QSpinBox::up-button {
        subcontrol-origin: border;
        subcontrol-position: top right;
        width:65px;
      }

      QCalendarWidget QSpinBox::down-button {
        subcontrol-origin: border;
        subcontrol-position: bottom right;
        width:65px;
      }

      QCalendarWidget QSpinBox::up-arrow {
        width:56px;
        height:56px;
      }

      QCalendarWidget QSpinBox::down-arrow {
        width:56px;
        height:56px;
      }
   
      /* header row */
      QCalendarWidget QWidget {
        alternate-background-color: #FFFFFF;
        font-family: Roboto;
        font-size: 19px;
      }
   
      /* normal days */
      QCalendarWidget QAbstractItemView:enabled {
  	    font-size:24px;  
  	    color: rgb(180, 180, 180);  
  	    background-color: black;
        height: 10px; 
        outline: none;
  	    selection-background-color: rgb(64, 64, 64); 
  	    selection-color: rgb(0, 255, 0); 
      }
   
      /* days in other months */
      /* navigation bar */
      QCalendarWidget QWidget#qt_calendar_navigationbar { 
        background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
          stop: 0 #cccccc, stop: 1 #333333); 
      }

      QCalendarWidget QAbstractItemView:disabled {
        color: rgb(64, 64, 64); 
        outline: none;
      }
		)").arg(scale_height(12)));
}
