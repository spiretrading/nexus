#include "Spire/Ui/IntegerInputWidget.hpp"
#include <algorithm>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto BUTTON_X() {
    static auto x = scale_width(12);
    return x;
  }
}

IntegerInputWidget::IntegerInputWidget(int value, QWidget* parent)
    : DecimalInputWidget(value, parent) {
  setDecimals(0);
  DecimalInputWidget::connect_change_signal([=] (auto value) {
    m_change_signal(static_cast<int>(value));
  });
  DecimalInputWidget::connect_submit_signal([=] (auto value) {
    m_submit_signal(static_cast<int>(value));
  });
  connect(lineEdit(), &QLineEdit::textEdited, this,
    &IntegerInputWidget::on_text_edited);
}

connection IntegerInputWidget::connect_change_signal(
    const ValueSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

connection IntegerInputWidget::connect_submit_signal(
    const ValueSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

//bool IntegerInputWidget::eventFilter(QObject* watched, QEvent* event) {
//  if(event->type() == QEvent::MouseButtonPress) {
//    auto e = static_cast<QMouseEvent*>(event);
//    if(e->button() == Qt::LeftButton && e->pos().x() < width() - BUTTON_X()) {
//      selectAll();
//      return true;
//    }
//  }
//  return QSpinBox::eventFilter(watched, event);
//}

void IntegerInputWidget::on_text_edited(const QString& text) {
  if(text.contains(QLocale().decimalPoint())) {
    lineEdit()->setText(lineEdit()->text().remove(QLocale().decimalPoint()));
  }
}
