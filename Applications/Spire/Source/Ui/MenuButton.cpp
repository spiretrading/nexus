#include "Spire/Ui/MenuButton.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/BoxPainter.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto MINIMUM_MENU_WIDTH() {
    static auto width = scale_width(130);
    return width;
  }
}

MenuButton::MenuButton(QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_body(&body),
      m_timer(this),
      m_is_mouse_down_on_button(false),
      m_menu_border_size(0) {
  setFocusPolicy(Qt::StrongFocus);
  enclose(*this, *m_body);
  m_menu = new ContextMenu(*this);
  m_menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_menu_window = static_cast<OverlayPanel*>(m_menu->window());
  m_menu_window->layout()->itemAt(0)->widget()->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_menu_window->set_positioning(OverlayPanel::Positioning::PARENT);
  m_menu_window->installEventFilter(this);
  m_timer.setSingleShot(true);
  on_menu_window_style();
  connect_style_signal(*m_menu_window, [=] { on_menu_window_style(); });
}

QWidget& MenuButton::get_body() {
  return *m_body;
}

ContextMenu& MenuButton::get_menu() {
  return *m_menu;
}

bool MenuButton::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Hide) {
    unmatch(*this, Press());
  } else if(event->type() == QEvent::MouseButtonPress) {
    auto& mouse_event = *static_cast<QMouseEvent*>(event);
    if(mouse_event.button() == Qt::LeftButton) {
      if(rect().contains(mapFromGlobal(mouse_event.globalPos()))) {
        m_is_mouse_down_on_button = true;
        match(*this, Press());
      }
    }
  } else if(event->type() == QEvent::MouseButtonRelease) {
    auto& mouse_event = *static_cast<QMouseEvent*>(event);
    if(mouse_event.button() == Qt::LeftButton) {
      if(mouse_event.source() == Qt::MouseEventSynthesizedByApplication) {
        return true;
      }
      auto is_on_button =
        rect().contains(mapFromGlobal(mouse_event.globalPos()));
      auto forward_mouse_event = [&] (QWidget* widget, QEvent::Type type) {
        auto position = widget->mapFromGlobal(mouse_event.globalPos());
        auto mouse_press_event = QMouseEvent(type, position,
          mouse_event.windowPos(), mouse_event.screenPos(),
          mouse_event.button(), mouse_event.buttons(),
          mouse_event.modifiers(), Qt::MouseEventSynthesizedByApplication);
        QCoreApplication::sendEvent(widget, &mouse_press_event);
      };
      if(is_on_button) {
        unmatch(*this, Press());
      }
      if(m_is_mouse_down_on_button) {
        if(is_on_button && m_timer.isActive()) {
          m_timer.stop();
        } else {
          if(auto child = m_menu->childAt(
              m_menu->mapFromGlobal(mouse_event.globalPos()))) {
            forward_mouse_event(child, QEvent::MouseButtonPress);
            forward_mouse_event(child, QEvent::MouseButtonRelease);
          } else {
            m_menu->hide();
          }
        }
      }
      m_is_mouse_down_on_button = false;
    }
  } else if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    if(key_event.key() == Qt::Key_Space) {
      if(m_menu->focusProxy() == m_menu->focusWidget()) {
        match(*this, Press());
        m_menu->hide();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void MenuButton::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter ||
      event->key() == Qt::Key_Return) {
    match(*this, Press());
    show_menu();
  }
  QWidget::keyPressEvent(event);
}

void MenuButton::keyReleaseEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter ||
      event->key() == Qt::Key_Return) {
    unmatch(*this, Press());
  }
  QWidget::keyReleaseEvent(event);
}

void MenuButton::mousePressEvent(QMouseEvent* event) {
  if(!m_is_mouse_down_on_button && event->button() == Qt::LeftButton) {
    m_is_mouse_down_on_button = true;
    match(*this, Press());
    show_menu();
    m_timer.start(500);
  }
  QWidget::mousePressEvent(event);
}

void MenuButton::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    unmatch(*this, Press());
    m_is_mouse_down_on_button = false;
  }
  QWidget::mouseReleaseEvent(event);
}

void MenuButton::show_menu() {
  m_menu->show();
  auto margins = m_menu_window->layout()->contentsMargins();
  auto max_width = std::max(
    {MINIMUM_MENU_WIDTH(), width(), m_menu->sizeHint().width()});
  auto window_width = max_width + margins.left() + margins.right();
  if(max_width == m_menu->sizeHint().width()) {
    window_width += m_menu_border_size;
  }
  m_menu_window->setFixedWidth(window_width);
}

void MenuButton::on_menu_window_style() {
  m_menu_border_size = 0;
  auto& stylist = find_stylist(*m_menu_window);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_menu_border_size += size;
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_menu_border_size += size;
        });
      });
  }
}
