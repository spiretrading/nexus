#include "Spire/Ui/MenuButton.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/ContextMenu.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Tooltip.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  using Label = StateSelector<void, struct LabelSelectorTag>;

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
  match(*m_body, Body());
  enclose(*this, *m_body);
  m_menu = new ContextMenu(*this);
  m_menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  link(*this, *m_menu);
  m_menu_window = static_cast<OverlayPanel*>(m_menu->window());
  m_menu_window->set_closed_on_focus_out(false);
  m_menu_window->layout()->itemAt(0)->widget()->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_menu_window->set_positioning(OverlayPanel::Positioning::PARENT);
  m_menu_window->installEventFilter(this);
  m_timer.setSingleShot(true);
  on_menu_window_style();
  connect_style_signal(
    *m_menu_window, std::bind_front(&MenuButton::on_menu_window_style, this));
}

QWidget& MenuButton::get_body() {
  return *m_body;
}

ContextMenu& MenuButton::get_menu() {
  return *m_menu;
}

bool MenuButton::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Hide) {
    unmatch(*this, PopUp());
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
            unmatch(*this, PopUp());
            m_menu_window->hide();
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
        unmatch(*this, PopUp());
        m_menu_window->hide();
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
  m_menu_window->show();
  match(*this, PopUp());
  update_menu_width();
}

void MenuButton::update_menu_width() {
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
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_menu_border_size += size;
          *has_update = true;
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_menu_border_size += size;
          *has_update = true;
        });
      });
  }
  if(*has_update && m_menu->isVisible()) {
    update_menu_width();
  }
}

MenuButton* Spire::make_menu_icon_button(QImage icon, QWidget* parent) {
  return make_menu_icon_button(std::move(icon), "", parent);
}

MenuButton* Spire::make_menu_icon_button(QImage icon, QString tooltip,
    QWidget* parent) {
  auto action_icon = new Icon(std::move(icon));
  action_icon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto arrow_icon =
    new Icon(imageFromSvg(":/Icons/dropdown-arrow.svg", scale(6, 4)));
  arrow_icon->setFixedSize(scale(6, 4));
  auto body = new QWidget();
  auto layout = make_hbox_layout(body);
  layout->addWidget(action_icon);
  layout->addWidget(arrow_icon);
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border(scale_width(1), QColor(Qt::transparent))).
      set(PaddingRight(scale_width(2)));
  });
  auto menu_button = new MenuButton(*box);
  add_tooltip(std::move(tooltip), *menu_button);
  update_style(*menu_button, [] (auto& style) {
    style.get((Press() || Hover() || FocusIn()) > Body()).
      set(BackgroundColor(QColor(0xE0E0E0)));
    style.get(FocusVisible() > Body()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Any() > Body() > Body() > is_a<Icon>()).
      set(Fill(QColor(0x535353)));
    style.get(Hover() > Body() > Body() > is_a<Icon>()).
      set(Fill(QColor(0x4B23A0)));
    style.get(Press() > Body() > Body() > is_a<Icon>()).
      set(Fill(QColor(0x7E71B8)));
    style.get(FocusIn() > Body() > Body() > is_a<Icon>()).
      set(Fill(QColor(0x684BC7)));
    style.get(Disabled() > Body() > Body() > is_a<Icon>()).
      set(Fill(QColor(0xC8C8C8)));
  });
  return menu_button;
}

MenuButton* Spire::make_menu_label_button(QString label, QWidget* parent) {
  auto button_label = make_label(std::move(label));
  button_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  match(*button_label, Label());
  update_style(*button_label, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border(scale_width(1), QColor(0xC8C8C8))).
      set(vertical_padding(scale_height(5))).
      set(PaddingLeft(scale_width(8))).
      set(PaddingRight(scale_width(14)));
  });
  auto icon_layer = new QWidget();
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
  layers->add(button_label);
  layers->add(icon_layer);
  auto menu_button = new MenuButton(*layers);
  update_style(*menu_button, [] (auto& style) {
    style.get(Any() > Body() > is_a<Icon>()).
      set(Fill(QColor(0x333333)));
    style.get((Hover() || Press() || FocusIn()) > Body() > is_a<Icon>()).
      set(Fill(QColor(Qt::black)));
    style.get(Disabled() > Body() > is_a<Icon>()).
      set(Fill(QColor(0xC8C8C8)));
    style.get((FocusIn() ||
      FocusVisible() || Hover() || PopUp() || Press()) > Body() > Label()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Disabled() > Body() > Label()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(TextColor(QColor(0xB8B8B8))).
      set(border_color(QColor(0xEBEBEB)));
  });
  return menu_button;
}
