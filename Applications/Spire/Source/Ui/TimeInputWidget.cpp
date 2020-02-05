#include "Spire/Ui/TimeInputWidget.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropDownMenu.hpp"

using namespace Spire;

TimeInputWidget::TimeInputWidget(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(scale_width(8));
  layout->setContentsMargins({});
  m_time_edit = new QTimeEdit(this);
  m_time_edit->setFixedHeight(scale_height(26));
  m_time_edit->setDisplayFormat("hh:mm");
  m_time_edit->setButtonSymbols(QAbstractSpinBox::NoButtons);
  m_time_edit->setMaximumTime({12, 59});
  m_time_edit->setMinimumTime({1, 00});
  m_time_edit->setStyleSheet(QString(R"(
    QTimeEdit {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %3px;
      padding-left: %4px;
    }

    QTimeEdit:focus {
      border: %1px solid #4B23A0 %2px solid #4B23A0;
    })").arg(scale_height(1)).arg(scale_width(1)).arg(scale_height(12))
        .arg(scale_width(8)));
  layout->addWidget(m_time_edit);
  m_drop_down_menu = new DropDownMenu({tr("PM"), tr("AM")}, this);
  m_drop_down_menu->setFixedHeight(scale_height(26));
  layout->addWidget(m_drop_down_menu);
}
