#include "Spire/Ui/DropDownWindow.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

DropDownWindow::DropDownWindow(bool is_click_activated, QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_widget(nullptr),
      m_is_click_activated(is_click_activated) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = new DropShadow(true, false, this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(scale_width(1), scale_height(1),
    scale_width(1), scale_height(1));
  parent->installEventFilter(this);
  parent->window()->installEventFilter(this);
  hide();
}

bool DropDownWindow::event(QEvent* event) {
  if(event->type() == QEvent::WindowDeactivate &&
      focusWidget() != nullptr &&
      !isAncestorOf(focusWidget())) {
    hide();
  }
  return QWidget::event(event);
}

bool DropDownWindow::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_widget) {
    if(event->type() == QEvent::Resize) {
      resize(m_widget->width() + 2, m_widget->height() + 2);
    }
  } else if(watched == parent()) {
    if(event->type() == QEvent::Move) {
      move_to_parent();
    } else if(event->type() == QEvent::FocusOut && !isActiveWindow()) {
      static_cast<QWidget*>(parent())->update();
      hide();
    } else if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Escape) {
        hide();
      } else if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        if(m_is_click_activated) {
          if(isVisible()) {
            // TODO: this hide just hides the dropdown window, not the drop shadow
            hide();
          } else {
            move_to_parent();
            show();
          }
          return true;
        }
      }
    } else if(event->type() == QEvent::MouseButtonPress &&
        m_is_click_activated) {
      auto e = static_cast<QMouseEvent*>(event);
      if(e->button() == Qt::LeftButton) {
        if(isVisible()) {
          hide();
        } else {
          move_to_parent();
          show();
        }
        return true;
      }
    }
  } else if(watched == static_cast<QWidget*>(parent())->window()) {
    if(event->type() == QEvent::WindowDeactivate && !isActiveWindow()) {
      hide();
    } else if(event->type() == QEvent::Move) {
      move_to_parent();
    } else if(event->type() == QEvent::MouseButtonPress) {
      auto e = static_cast<QMouseEvent*>(event);
      if(e->button() == Qt::LeftButton) {
        hide();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DropDownWindow::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    hide();
  }
}

void DropDownWindow::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.setPen(QColor("#C8C8C8"));
  painter.drawRect(0, 0, width() - 1, height() - 1);
  painter.setPen("#4B23A0");
  painter.drawLine(0, 0, static_cast<QWidget*>(parent())->width() - 1, 0);
}

void DropDownWindow::set_widget(QWidget* widget) {
  if(m_widget != nullptr) {
    return;
  }
  m_widget = widget;
  m_widget->installEventFilter(this);
  m_widget->setFocusProxy(this);
  layout()->addWidget(m_widget);
}

void DropDownWindow::move_to_parent() {
  auto parent_widget = static_cast<QWidget*>(parent());
  auto pos = parent_widget->window()->mapToGlobal(
    parent_widget->frameGeometry().bottomLeft());
  move(pos);
  raise();
}
