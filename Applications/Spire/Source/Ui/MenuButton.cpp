#include "Spire/Ui/MenuButton.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  using Label = StateSelector<void, struct LabelSelectorTag>;
}

MenuButton::MenuButton(QWidget& body, OverlayPanel& menu, QWidget* parent)
    : QWidget(parent),
      m_menu(&menu),
      m_timer(this),
      m_is_mouse_down_on_button(false) {
  setFocusPolicy(Qt::StrongFocus);
  enclose(*this, body);
  m_menu->setWindowFlags(Qt::Popup | (m_menu->windowFlags() & ~Qt::Tool));
  m_menu->set_positioning(OverlayPanel::Positioning::PARENT);
  m_menu->installEventFilter(this);
  m_timer.setSingleShot(true);
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
      auto& menu_body = m_menu->get_body();
      if(menu_body.focusProxy() == menu_body.focusWidget()) {
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
    m_menu->show();
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
    m_menu->show();
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

MenuButton* Spire::make_menu_label_button(QString label, OverlayPanel& menu,
    QWidget* parent) {
  auto label_box = make_label(std::move(label));
  label_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  label_box->setFixedHeight(scale_height(26));
  match(*label_box, Label());
  update_style(*label_box, [] (auto& style) {
    style.get(Any()).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(vertical_padding(scale_height(5))).
      set(PaddingLeft(scale_width(8))).
      set(PaddingRight(scale_width(14)));
  });
  auto icon_layer = new QWidget();
  icon_layer->setAttribute(Qt::WA_TransparentForMouseEvents);
  icon_layer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto arrow_icon =
    new Icon(imageFromSvg(":/Icons/dropdown-arrow.svg", scale(6, 4)));
  arrow_icon->setFixedSize(scale(6, 4));
  auto icon_layer_layout = make_hbox_layout(icon_layer);
  icon_layer_layout->addStretch();
  icon_layer_layout->addWidget(arrow_icon);
  icon_layer_layout->addSpacing(scale_width(8));
  auto layers = new LayeredWidget();
  layers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layers->add(label_box);
  layers->add(icon_layer);
  auto menu_button = new MenuButton(*layers, menu);
  menu_button->setFixedHeight(scale_height(26));
  menu.setParent(menu_button);
  update_style(*menu_button, [] (auto& style) {
    style.get(Any() > is_a<Icon>()).
      set(Fill(QColor(0x333333)));
    style.get((Hover() || Press() || FocusIn()) > is_a<Icon>()).
      set(Fill(QColor(0xFFFFFF)));
    style.get(Disabled() > is_a<Icon>()).
      set(Fill(QColor(0xC8C8C8)));
    style.get(FocusVisible() > Label()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Hover() > Label()).
      set(BackgroundColor(QColor(0x4B23A0))).
      set(TextColor(QColor(0xFFFFFF)));
    style.get(Press() > Label()).
      set(BackgroundColor(QColor(0x7E71B8))).
      set(TextColor(QColor(0xFFFFFF)));
    style.get(FocusIn() > Label()).
      set(BackgroundColor(QColor(0x684BC7))).
      set(TextColor(QColor(0xFFFFFF)));
    style.get(Disabled() > Label()).
      set(TextColor(QColor(0xB8B8B8)));
    //style.get(FocusVisible() > (is_a<TextBox>() && !(+Any() << is_a<OverlayPanel>()))).
    //  set(border_color(QColor(0x4B23A0)));
    //style.get(Hover() > (is_a<TextBox>() && !(+Any() << is_a<OverlayPanel>()))).
    //  set(BackgroundColor(QColor(0x4B23A0))).
    //  set(TextColor(QColor(0xFFFFFF)));
    //style.get(Press() > (is_a<TextBox>() && !(+Any() << is_a<OverlayPanel>()))).
    //  set(BackgroundColor(QColor(0x7E71B8))).
    //  set(TextColor(QColor(0xFFFFFF)));
    //style.get(FocusIn() > (is_a<TextBox>() && !(+Any() << is_a<OverlayPanel>()))).
    //  set(BackgroundColor(QColor(0x684BC7))).
    //  set(TextColor(QColor(0xFFFFFF)));
    //style.get(Disabled() > (is_a<TextBox>() && !(+Any() << is_a<OverlayPanel>()))).
    //  set(TextColor(QColor(0xB8B8B8)));
  });
  return menu_button;
}