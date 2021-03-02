#include "Spire/Ui/Button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

Button::Button(QWidget* component, QWidget* parent)
    : QWidget(parent),
      m_component(component),
      m_is_down(false) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_component);
  m_component->installEventFilter(this);
  setFocusPolicy(Qt::StrongFocus);
  setObjectName("Button");
  setStyleSheet(QString(R"(
    #Button {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8;
    }
    #Button:hover {
      border-color: #4B23A0;
    }
    #Button:focus {
      border-color: #4B23A0;
    }
    #Button:disabled {
      background-color: #F5F5F5;
      border-color: #C8C8C8;
    })").arg(scale_width(1)));
}

connection Button::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

bool Button::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_component) {
    switch(event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
      event->ignore();
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void Button::focusOutEvent(QFocusEvent* event) {
  if(event->reason() != Qt::PopupFocusReason && m_is_down) {
    m_is_down = false;
  }
  QWidget::focusOutEvent(event);
}

void Button::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(!event->isAutoRepeat()) {
        m_clicked_signal();
      }
      break;
    case Qt::Key_Space:
      if(!event->isAutoRepeat()) {
        m_is_down = true;
      }
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void Button::keyReleaseEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Space:
      if(!event->isAutoRepeat() && m_is_down) {
        m_is_down = false;
        m_clicked_signal();
      }
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void Button::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && rect().contains(event->pos())) {
    m_is_down = true;
  }
  QWidget::mousePressEvent(event);
}

void Button::mouseReleaseEvent(QMouseEvent* event) {
  if(m_is_down && event->button() == Qt::LeftButton &&
      rect().contains(event->pos())) {
    m_is_down = false;
    m_clicked_signal();
  }
  QWidget::mouseReleaseEvent(event);
}

Button* Spire::make_label_button(const QString& label, const QFont& font,
    QWidget* parent) {
  auto text_box = new TextBox(label);
  text_box->setReadOnly(true);
  text_box->setAlignment(Qt::AlignCenter);
  text_box->setFont(font);
  text_box->setFocusPolicy(Qt::NoFocus);
  text_box->setContextMenuPolicy(Qt::NoContextMenu);
  text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto button = new Button(text_box, parent);
  button->setStyleSheet(QString(R"(
    #Button {
      background-color: #EBEBEB;
      border: none;
    }
    #Button:hover {
      background-color: #4B23A0;
      color: #FFFFFF;
    }
    #Button:focus {
      border: %1px solid #4B23A0;
    }
    #Button:disabled {
      background-color: #EBEBEB;
      border: none;
    })").arg(scale_width(1)));
  return button;
}
