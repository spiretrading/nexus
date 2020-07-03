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

IntegerInputWidget::IntegerInputWidget(int min_value, int max_value,
    QWidget* parent)
    : QSpinBox(parent),
      m_min_value(min_value),
      m_max_value(max_value) {
  installEventFilter(this);
  setMinimum(m_min_value);
  setMaximum(m_max_value);
  setStyleSheet(QString(R"(
    QSpinBox {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8;
      color: #000000;
      font-family: Roboto;
      font-size: %5px;
      padding-left: %4px;
    }

    QSpinBox:focus {
      border: %1px solid #4B23A0;
    }

    QSpinBox::up-button {
      border: none;
    }

    QSpinBox::down-button {
      border: none;
    }

    QSpinBox::up-arrow {
      height: %2px;
      image: url(:/Icons/arrow-up.svg);
      padding-top: %6px;
      width: %3px;
    }

    QSpinBox::down-arrow {
      height: %2px;
      image: url(:/Icons/arrow-down.svg);
      width: %3px;
    })").arg(scale_width(1)).arg(scale_height(6)).arg(scale_width(6))
        .arg(scale_width(10)).arg(scale_height(12)).arg(scale_height(4)));
  setContextMenuPolicy(Qt::NoContextMenu);
  findChild<QLineEdit*>()->installEventFilter(this);
}

bool IntegerInputWidget::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::MouseButtonPress) {
    auto e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::LeftButton && e->pos().x() < width() - BUTTON_X()) {
      selectAll();
      return true;
    }
  }
  return QSpinBox::eventFilter(watched, event);
}

void IntegerInputWidget::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    setValue(text().toInt());
    if(value() < m_min_value) {
      setValue(m_min_value);
    }
    Q_EMIT valueChanged(value());
  } else if(event->key() == Qt::Key_Down && value() <= m_min_value) {
    setValue(m_min_value);
    Q_EMIT valueChanged(value());
  } else {
    QSpinBox::keyPressEvent(event);
  }
}
